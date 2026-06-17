#include "driver/uart.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

/*************************************************************************************************************************/
// Khởi tạo handle: request_data_timer
TimerHandle_t request_data_timer;
#define REQUEST_DATA_PERIOD 30 // Milli giây

/*************************************************************************************************************************/
// Header: Giao tiếp với WinForm (VS) qua UART0 mặc định
#define DOWNLINK_START_BYTE 0xBB
#define END_BYTE 0xED
#define LONG_FRAME_LENGTH 12
#define SHORT_FRAME_LENGTH 9
#define MAX_FRAME_LENGTH LONG_FRAME_LENGTH
#define UART_READING 1
#define UART_WAIT 0

#pragma pack(1)
typedef struct {
  byte rx_buff[MAX_FRAME_LENGTH];
  uint8_t rx_index;
  uint8_t rx_byte;
  uint8_t state;
  uint8_t length_expected;
} UartRxData_t; // Struct của biến thực hiện nhận dữ liệu từ UART0
#pragma pack()

UartRxData_t uart_rx;       // Biến thực hiện nhận dữ liệu từ WinForm (VS)
uint8_t g_uart_rx_flag = 0; // Cờ báo đọc thành công dữ liệu từ VS

/*************************************************************************************************************************/
// Header: Giao tiếp với ESP32 Slave qua ESP-NOW
#define UPLINK_START_BYTE 0xAA
#define END_BYTE 0xED
#define EMPTY_ID 0x0E        // Mã từ ESP32 Slave báo trống dữ liệu cho Master
#define STM32_DATA_ID 0xDA   // Mã yêu cầu Slave gửi dữ liệu STM32 lên
#define EMPTY_FRAME_LENGTH 3 // Độ dài frame báo trống dữ liệu từ Slave
#define DATA_FRAME_LENGTH 18 // Độ dài frame có dữ liệu từ Slave
#define MAC_LEN 6            // Độ dài khung địa chỉ MAC

#pragma pack(1)
typedef struct {
  int8_t start_check_value;
  int16_t x_angle;
  int16_t z_angle;
  int16_t x_angular_velocity;
  int16_t z_angular_velocity;
  int16_t x_coordinate;
  int16_t left_wheel_torque;
  int16_t right_wheel_torque;
  int16_t checksum_crc16;
  int8_t end_check_value;
} SlaveDataRc_t; // Cấu trúc dữ liệu nhận từ Slave
#pragma pack()

SlaveDataRc_t slave_data_rc;                                    // Biến lưu dữ liệu STM32 nhận từ Slave
uint8_t g_request_timer_flag = 0;                               // Cờ timer yêu cầu Slave gửi dữ liệu
uint8_t slave_address[] = {0x78, 0xE3, 0x6D, 0xDD, 0xD9, 0x48}; // Địa chỉ MAC của Slave
esp_now_peer_info_t peer_info;                                  // Biến lưu thông tin thiết bị nhận

uint8_t frame_request_stm32data[LONG_FRAME_LENGTH] = {
    DOWNLINK_START_BYTE, STM32_DATA_ID, STM32_DATA_ID, STM32_DATA_ID, STM32_DATA_ID, STM32_DATA_ID, STM32_DATA_ID,
    STM32_DATA_ID,       STM32_DATA_ID, STM32_DATA_ID, STM32_DATA_ID, END_BYTE}; // Frame yêu cầu Slave gửi dữ liệu STM32 lên

/*************************************************************************************************************************/
// Hàm tính mã Modbus CRC16
uint16_t ModbusCRC16(uint8_t *p_data, uint8_t pos_start, uint8_t pos_end) {
  uint16_t crc = 0xFFFF;
  for (uint8_t pos = pos_start; pos <= pos_end; pos++) {
    crc ^= (uint16_t)p_data[pos];
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

/*************************************************************************************************************************/
// State-Machine đọc dữ liệu từ WinForm (VS) qua UART0
void ReadVSData(UartRxData_t *p_rx_data) {
  while (Serial.available() > 0) // Kiểm tra buffer UART có byte nào không
  {
    p_rx_data->rx_byte = Serial.read();
    switch (p_rx_data->state) {
    case UART_WAIT: {
      p_rx_data->rx_buff[0] = p_rx_data->rx_byte; // Tìm start byte
      if (p_rx_data->rx_buff[0] == DOWNLINK_START_BYTE) {
        p_rx_data->state = UART_READING; // Chuyển sang trạng thái đọc
        p_rx_data->length_expected = SHORT_FRAME_LENGTH;
        p_rx_data->rx_index = 1;
      }
      break;
    }
    case UART_READING: {
      p_rx_data->rx_buff[p_rx_data->rx_index] = p_rx_data->rx_byte;
      if (p_rx_data->rx_index == 2 && p_rx_data->rx_buff[2] == 0x01) {
        p_rx_data->length_expected = LONG_FRAME_LENGTH;
      }
      if (p_rx_data->rx_index == p_rx_data->length_expected - 1) {
        // Kiểm tra end byte
        if (p_rx_data->rx_buff[p_rx_data->rx_index] == END_BYTE) {
          // Kiểm tra Checksum
          uint16_t crc16 = ModbusCRC16(p_rx_data->rx_buff, 0, p_rx_data->length_expected - 4);
          uint8_t low_byte_crc16 = (uint8_t)(crc16 & 0xFF);
          uint8_t high_byte_crc16 = (uint8_t)((crc16 >> 8) & 0xFF);
          if (low_byte_crc16 == p_rx_data->rx_buff[p_rx_data->length_expected - 3] &&
              high_byte_crc16 == p_rx_data->rx_buff[p_rx_data->length_expected - 2]) {
            // Checksum đúng -> Bật cờ thông báo và reset state machine
            g_uart_rx_flag = 1;
            p_rx_data->rx_index = 0;
            p_rx_data->state = UART_WAIT;
          } else {
            // Checksum sai -> Reset state machine
            p_rx_data->rx_index = 0;
            p_rx_data->state = UART_WAIT;
          }
        } else {
          // End byte sai -> Reset state machine
          p_rx_data->rx_index = 0;
          p_rx_data->state = UART_WAIT;
        }
      } else {
        p_rx_data->rx_index++; // Tăng index
      }
      break;
    }
    }
  }
}

/*************************************************************************************************************************/
// Hàm gửi lệnh xuống ESP32 Slave qua ESP-NOW
void ESPNOWSendToSlave(uint8_t *p_data) {
  uint8_t data_length = (p_data[2] == 0x00) ? SHORT_FRAME_LENGTH : LONG_FRAME_LENGTH;
  esp_now_send(slave_address, (uint8_t *)p_data, data_length);
}

/*************************************************************************************************************************/
// Hàm callback khi nhận dữ liệu từ ESP-NOW
void ESPNOWCallback(const esp_now_recv_info_t *p_esp_now_info, const uint8_t *p_esp_data, int data_len) {
  // Kiểm tra danh tính nguồn gửi đến
  if (memcmp(p_esp_now_info->src_addr, slave_address, MAC_LEN)) {
    return;
  }

  // Xử lý frame
  if (data_len == DATA_FRAME_LENGTH) {
    // Data frame -> Gửi dữ liệu STM32 lên WinForm (VS)
    memcpy(&slave_data_rc, p_esp_data, DATA_FRAME_LENGTH);
    Serial.write((uint8_t *)&slave_data_rc, DATA_FRAME_LENGTH);
  }
  if (data_len == EMPTY_FRAME_LENGTH) {
    // Empty frame -> Không gửi
  }
}

/*************************************************************************************************************************/
// Hàm callback của request_data_timer
void TimerCallback(TimerHandle_t xTimer) { g_request_timer_flag = 1; }

/*************************************************************************************************************************/
// Hàm khởi tạo ESP-NOW
void InitESPNOW(void) {
  // Cố định kênh Wifi 1
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  // Khởi tạo ESP-NOW
  esp_now_init();
  // Đăng ký hàm callback nhận dữ liệu từ ESP-NOW
  esp_now_register_recv_cb((esp_now_recv_cb_t)ESPNOWCallback);
  // Cấu hình ESP-NOW
  memcpy(peer_info.peer_addr, slave_address, MAC_LEN);
  peer_info.channel = 1;
  peer_info.encrypt = false;
  // Thêm thiết bị nhận
  esp_now_add_peer(&peer_info);
}

/*************************************************************************************************************************/
void setup() {
  // Khởi tạo trạng thái state machine
  uart_rx.state = UART_WAIT;
  // Khởi tạo UART0 giao tiếp WinForm
  Serial.begin(115200);
  // Thiết lập esp32 là một trạm thu phát wi-fi
  WiFi.mode(WIFI_STA);
  // Khởi tạo ESP-NOW
  InitESPNOW();
  // Khởi tạo Timer yêu cầu Slave gửi dữ liệu STM32
  request_data_timer = xTimerCreate("Timer yêu cầu Slave gửi dữ liệu", REQUEST_DATA_PERIOD, pdTRUE, NULL, TimerCallback);
  // Kích hoạt Timer
  xTimerStart(request_data_timer, 0);
}

void loop() {
  // Yêu cầu Slave gửi dữ liệu STM32
  if (g_request_timer_flag == 1) {
    // Yêu cầu Slave gửi dữ liệu STM32
    ESPNOWSendToSlave(frame_request_stm32data);
    g_request_timer_flag = 0;
  }

  // Đọc dữ liệu từ WinForm (VS)
  ReadVSData(&uart_rx);
  if (g_uart_rx_flag == 1) {
    // Chuyển tiếp gói tin xuống Slave
    ESPNOWSendToSlave(uart_rx.rx_buff);
    g_uart_rx_flag = 0;
  }

  vTaskDelay(pdMS_TO_TICKS(1));
}

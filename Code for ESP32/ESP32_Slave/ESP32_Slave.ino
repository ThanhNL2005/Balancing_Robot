#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <esp_now.h>

/******************************************************************************/
// Khởi tạo UART0
#define RX_PIN 3
#define TX_PIN 1
HardwareSerial my_serial(0);
// Khởi tạo handle: UART Mutex
SemaphoreHandle_t uart_mutex = NULL;
// Khởi tạo handle: ESP-NOW Queue
QueueHandle_t esp_now_queue = NULL;
// Khởi tạo handle: ESP-NOW Task
TaskHandle_t esp_now_task = NULL;

/******************************************************************************/
// Header: Giao tiếp với STM32 qua UART0
#define UPLINK_START_BYTE 0xAA
#define END_BYTE 0xED
#define RX_LENGTH 18
#define UART_READING 1
#define UART_WAIT 0

typedef struct {
  byte rx_buff[RX_LENGTH];
  uint8_t rx_index;
  uint8_t rx_byte;
  uint8_t state;
} UartRxData_t; // Struct của biến thực hiện nhận dữ liệu UART0

UartRxData_t uart_rx_buffer;
UartRxData_t uart_rx;
uint8_t g_rx_stm32_flag = 0; // Cờ báo nhận thành công dữ liệu STM32

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
} Stm32Data_t;             // Cấu trúc dữ liệu của STM32 Data
Stm32Data_t stm32_data_rc; // Biến lưu dữ liệu STM32
#pragma pack()

/******************************************************************************/
// Header: Giao tiếp với ESP32 Master qua ESP-NOW
#define MAX_FRAME_LEN 12
#define LONG_FRAME_LENGTH 12
#define SHORT_FRAME_LENGTH 9
#define MAC_LEN 6
#define STM32_DATA_ID 0x04 // Mã từ ESP32 Master yêu cầu gửi dữ liệu STM32 lên
#define EMPTY_ID 0xEE      // Mã từ ESP32 Slave báo trống dữ liệu

#pragma pack(1)
typedef struct {
  uint8_t first_check = UPLINK_START_BYTE;
  uint8_t id_number = EMPTY_ID;
  uint8_t last_check = END_BYTE;
} EmptyFrame_t;
EmptyFrame_t
    empty_frame; // Frame gửi cho Master khi không đọc được dữ liệu STM32
#pragma pack()

uint8_t master_address[] = {
    0xA0, 0xDD, 0x6C,
    0x02, 0xCF, 0x94};         // Địa chỉ MAC của thiết bị nhận (ESP32 Master)
esp_now_peer_info_t peer_info; // Biến lưu thông tin thiết bị nhận

/******************************************************************************/
// Hàm tính mã Modbus CRC16
uint16_t modbusCrc16(uint8_t *data, uint16_t length) {
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = 0; pos < length; pos++) {
    crc ^= (uint16_t)data[pos];
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

/******************************************************************************/
// Hàm đọc dữ liệu từ STM32 -> Lưu vào UART_Rx
void readStm32Data(UartRxData_t *p_rx_data) {
  if (p_rx_data == NULL) {
    return;
  }
  while (my_serial.available() > 0) {
    p_rx_data->rx_byte = my_serial.read();
    switch (p_rx_data->state) {
    case UART_WAIT: {
      p_rx_data->rx_buff[0] =
          p_rx_data->rx_byte; // Dò start byte để lấy điểm bắt đầu đọc
      if (p_rx_data->rx_buff[0] == UPLINK_START_BYTE) {
        p_rx_data->state = UART_READING; // Chuyển sang trạng thái đọc
        p_rx_data->rx_index = 1;
      } else {
        // Byte đầu tiên không phải UPLINK_START_BYTE
      }
      break;
    }

    case UART_READING: {
      p_rx_data->rx_buff[p_rx_data->rx_index] =
          p_rx_data->rx_byte; // Lưu từng byte
      // Kiểm tra end byte
      if (p_rx_data->rx_index == RX_LENGTH - 1) {
        if (p_rx_data->rx_buff[p_rx_data->rx_index] == END_BYTE) {
          // Kiểm tra Checksum
          uint16_t crc16 = modbusCrc16(p_rx_data->rx_buff, RX_LENGTH - 3);
          uint8_t low_byte_crc16 = (uint8_t)(crc16 & 0xFF);
          uint8_t high_byte_crc16 = (uint8_t)((crc16 >> 8) & 0xFF);
          if (low_byte_crc16 == p_rx_data->rx_buff[RX_LENGTH - 3] &&
              high_byte_crc16 == p_rx_data->rx_buff[RX_LENGTH - 2]) {
            // Checksum đúng -> Bật cờ thông báo và reset state machine
            if (xSemaphoreTake(uart_mutex, portMAX_DELAY) == pdTRUE) {
              memcpy(uart_rx.rx_buff, p_rx_data->rx_buff,
                     RX_LENGTH); // Lưu vào UART_Rx
              g_rx_stm32_flag =
                  1; // Dựng cờ báo đọc dữ liệu từ STM32 thành công
              xSemaphoreGive(uart_mutex);
            }
            p_rx_data->rx_index = 0;
            p_rx_data->state = UART_WAIT;
          } else {
            // Checksum sai -> Reset state machine
            p_rx_data->rx_index = 0;
            p_rx_data->state = UART_WAIT;
          }
        } else {
          // Frame bị mất END_BYTE -> Reset state machine
          p_rx_data->rx_index = 0;
          p_rx_data->state = UART_WAIT;
        }
      } else {
        p_rx_data->rx_index =
            p_rx_data->rx_index + 1; // Tăng index của mảng lưu
      }
      break;
    }
    }
  }
}

/******************************************************************************/
// Hàm callback khi nhận dữ liệu qua ESP-NOW
void espNowCallback(const esp_now_recv_info_t *esp_now_info,
                    const uint8_t *esp_data, int data_len) {
  // Kiểm tra danh tính nguồn gửi đến
  if (memcmp(esp_now_info->src_addr, master_address, MAC_LEN)) {
    return;
  }
  // Lưu dữ liệu vào ESPNOW Queue
  if (data_len == SHORT_FRAME_LENGTH) {
    uint8_t buff[MAX_FRAME_LEN] = {0};
    memcpy(buff, esp_data, data_len);
    xQueueSend(esp_now_queue, buff, 0);
  } else if (data_len == LONG_FRAME_LENGTH) {
    xQueueSend(esp_now_queue, esp_data, 0);
  }
}

/******************************************************************************/
// Hàm xử lý dữ liệu nhận từ ESP32 Master
void processMasterData(void *pvParameters) {
  uint8_t esp_buffer[MAX_FRAME_LEN];
  while (1) {
    if (xQueueReceive(esp_now_queue, esp_buffer, portMAX_DELAY) == pdTRUE) {
      if (esp_buffer[1] == STM32_DATA_ID) {
        // Thực hiện lệnh: Gửi dữ liệu STM32 định kỳ lên cho Master
        stm32DataToMaster(&uart_rx);
      } else {
        // Thục hiện lệnh: Chuyển tiếp gói tin từ WinForm (VS) xuống STM32
        masterDataToStm32(esp_buffer);
      }
    }
  }
}

/******************************************************************************/
// Hàm gửi dữ liệu STM32 lên Master
void stm32DataToMaster(UartRxData_t *p_rx_data) {
  if (xSemaphoreTake(uart_mutex, portMAX_DELAY) == pdTRUE) {
    if (g_rx_stm32_flag) {
      memcpy(&stm32_data_rc, p_rx_data->rx_buff, sizeof(stm32_data_rc));
      g_rx_stm32_flag = 0;
      xSemaphoreGive(uart_mutex);
      // Gửi dữ liệu STM32 cho Master
      esp_now_send(master_address, (uint8_t *)&stm32_data_rc,
                   sizeof(stm32_data_rc));
    } else {
      xSemaphoreGive(uart_mutex);
      // Gửi frame báo trống dữ liệu cho Master
      esp_now_send(master_address, (uint8_t *)&empty_frame,
                   sizeof(empty_frame));
    }
  }
}

/******************************************************************************/
// Hàm gửi dữ liệu từ Master xuống STM32
void masterDataToStm32(uint8_t *buff) {
  uint8_t buff_len = buff[2] == 0x00 ? SHORT_FRAME_LENGTH : LONG_FRAME_LENGTH;
  my_serial.write(buff, buff_len);
}

/******************************************************************************/
// Hàm khởi tạo ESP-NOW
void initEspNow(void) {
  if (esp_now_init() != ESP_OK) {
    return;
  }
  // Đăng ký hàm callback trong ESPNOW
  esp_now_register_recv_cb((esp_now_recv_cb_t)espNowCallback);
  // Cấu hình ESP-NOW
  memcpy(peer_info.peer_addr, master_address,
         MAC_LEN);           // Gán địa chỉ thiết bị nhận
  peer_info.channel = 0;     // Sử dụng kênh Wifi mặc định 0
  peer_info.encrypt = false; // Không mã hoá
  // Thêm thiết bị nhận (ESP32 Master)
  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    return;
  }
}

/******************************************************************************/
#define TASK_PROCESS_MASTER_PRIO 5
#define TASK_PROCESS_MASTER_STACK 4096

void setup() {
  // Khởi tạo trạng thái state machine
  uart_rx_buffer.state = UART_WAIT;
  // Thiết lập esp32 là một trạm thu phát wi-fi
  WiFi.mode(WIFI_STA);
  // Khởi tạo phần cứng UART0
  my_serial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  // Khởi tạo UART Mutex
  uart_mutex = xSemaphoreCreateMutex();
  // Khởi tạo ESP-NOW Queue
  esp_now_queue = xQueueCreate(32, MAX_FRAME_LEN);
  // Khởi tạo Task xử lý dữ liệu từ Master
  xTaskCreatePinnedToCore(processMasterData, "Xu ly du lieu Master",
                          TASK_PROCESS_MASTER_STACK, // Dung lượng stack
                          NULL,
                          TASK_PROCESS_MASTER_PRIO, // Độ ưu tiên
                          &esp_now_task,
                          1); // Chạy trên Core 1
  // Khởi tạo ESP-NOW
  initEspNow();
}

void loop() {
  readStm32Data(&uart_rx_buffer);
  vTaskDelay(pdMS_TO_TICKS(1));
}
#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include "driver/uart.h"
#include <HardwareSerial.h>

/******************************************************************************/
// Khởi tạo handle: requestData_Timer
TimerHandle_t requestData_Timer;
#define REQUEST_DATA_PERIOD 30  // Milli giây

/******************************************************************************/
// Header: Giao tiếp với WinForm (VS) qua UART0 mặc định
#define DOWNLINK_START_BYTE 0xBB
#define END_BYTE 0xED
#define longFrameLength 12
#define shortFrameLength 9
#define maxFrameLength longFrameLength
#define UART_READING 1
#define UART_WAIT 0

#pragma pack(1)
typedef struct
{
  byte rxBuff[maxFrameLength];
  uint8_t rxIndex;
  uint8_t rxByte;
  uint8_t STATE;
} UART_RxData_t;  // Struct của biến thực hiện nhận dữ liệu từ UART0
#pragma pack()

UART_RxData_t UART_Rx;      // Biến thực hiện nhận dữ liệu từ WinForm (VS)
uint8_t g_UART_RxFlag = 0;  // Cờ báo đọc thành công dữ liệu từ VS

/******************************************************************************/
// Header: Giao tiếp với ESP32 Slave qua ESP-NOW
#define UPLINK_START_BYTE 0xAA
#define END_BYTE 0xED
#define EMPTY_ID 0xEE         // Mã từ ESP32 Slave báo trống dữ liệu cho Master
#define STM32DATA_ID 0x04     // Mã từ ESP32 Master yêu cầu Slave gửi dữ liệu STM32 lên
#define EMPTY_FRAME_LENGTH 3  // Độ dài frame báo trống dữ liệu từ Slave
#define DATA_FRAME_LENGTH 16  // Độ dài frame có dữ liệu từ Slave
#define MAC_LEN 6             // Độ dài khung địa chỉ MAC

#pragma pack(1)
typedef struct
{
  int8_t startCheckValue;
  int16_t xAngle;
  int16_t zAngle;
  int16_t xAngularVelocity;
  int16_t zAngularVelocity;
  int16_t xCoordinate;
  int16_t leftWheelTorque;
  int16_t rightWheelTorque;
  int8_t endCheckValue;
} Slave_DataRC_t;  // Cấu trúc dữ liệu nhận từ Slave
#pragma pack()

Slave_DataRC_t slaveDataRC;                                       // Biến lưu dữ liệu nhận từ Slave
uint8_t requestTimer_Flag = 0;                                    // Cờ timer yêu cầu Slave gửi dữ liệu
uint8_t SlaveAddress[] = { 0x78, 0xE3, 0x6D, 0xDD, 0xD9, 0x48 };  // Địa chỉ MAC của Slave
esp_now_peer_info_t peerInfo;                                     // Biến lưu thông tin thiết bị nhận

uint8_t frame_Request_STM32Data[longFrameLength] = {
  DOWNLINK_START_BYTE,
  STM32DATA_ID,
  0x01,
  0, 0, 0, 0, 0, 0, 0, 0,
  END_BYTE
};  // Frame yêu cầu Slave gửi dữ liệu STM32 lên

/******************************************************************************/
// Hàm tính mã Modbus CRC16
uint16_t Modbus_CRC16(uint8_t *data, uint8_t pos_start, uint8_t pos_end) {
  uint16_t crc = 0xFFFF;
  for (uint8_t pos = pos_start; pos <= pos_end; pos++) {
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
// Sate-Machine đọc dữ liệu từ WinForm (VS) qua UART0
void Read_VS_Data(UART_RxData_t *pRxData) {
  uint8_t UARTRxLength_expected = shortFrameLength;
  while (Serial.available() > 0)  // Kiểm tra buffer UART có byte nào không
  {
    pRxData->rxByte = Serial.read();
    switch (pRxData->STATE) {
      case UART_WAIT:
        {
          pRxData->rxBuff[0] = pRxData->rxByte;  // Tìm start byte
          if (pRxData->rxBuff[0] == DOWNLINK_START_BYTE) {
            pRxData->STATE = UART_READING;  // Chuyển sang trạng thái đọc
            //Serial.println("Đã chuyển sang trạng thái Reading");
            UARTRxLength_expected = shortFrameLength;
            pRxData->rxIndex = 1;
          } else {
            //Serial.println("Không thể chuyển sang trạng thái Reading");
          }
          break;
        }
      case UART_READING:
        {
          pRxData->rxBuff[pRxData->rxIndex] = pRxData->rxByte;
          if (pRxData->rxIndex == 2 && pRxData->rxBuff[2] == 0x01) {
            UARTRxLength_expected = longFrameLength;
          }
          if (pRxData->rxIndex == UARTRxLength_expected - 1) {
            // Kiểm tra end byte
            if (pRxData->rxBuff[pRxData->rxIndex] == END_BYTE) {
              /*Serial.println("Frame đọc được là: ");
              for (int i = 0; i <= pRxData->rxIndex; i++) {
                Serial.printf("%02X ", pRxData->rxBuff[i]);
              }
              Serial.println("\n");*/

              // Kiểm tra Checksum
              uint16_t CRC16 = Modbus_CRC16(pRxData->rxBuff, 0, UARTRxLength_expected - 4);
              uint8_t Low_Byte_CRC16 = (uint8_t)(CRC16 & 0xFF);
              uint8_t High_Byte_CRC16 = (uint8_t)((CRC16 >> 8) & 0xFF);
              if (Low_Byte_CRC16 == pRxData->rxBuff[UARTRxLength_expected - 3]
                  && High_Byte_CRC16 == pRxData->rxBuff[UARTRxLength_expected - 2]) {
                // Checksum đúng -> Bật cờ thông báo và reset state machine
                g_UART_RxFlag = 1;
                pRxData->rxIndex = 0;
                pRxData->STATE = UART_WAIT;
                /*Serial.print("Vị trí của byte checksum: ");
                Serial.println(UARTRxLength_expected - 2);  // Frame 8 bytes: 6, frame 11 bytes: 9
                Serial.println("Checksum đúng. Hoàn thành nhận dữ liệu.");
                Serial.print("Checksum nhận: ");
                Serial.println(pRxData->rxBuff[UARTRxLength_expected - 2]);
                Serial.print("Checksum tính toán: ");
                Serial.println(checksum);*/
              } else {
                // Checksum sai -> Reset state machine
                pRxData->rxIndex = 0;
                pRxData->STATE = UART_WAIT;
                /*Serial.print("Vị trí của byte checksum: ");
                Serial.println(UARTRxLength_expected - 2);  // Frame 8 bytes: 6, frame 11 bytes: 9
                Serial.println("Checksum sai. Frame sẽ bị loại bỏ.");
                Serial.print("Checksum nhận: ");
                Serial.println(pRxData->rxBuff[UARTRxLength_expected - 2]);
                Serial.print("Checksum tính toán: ");
                Serial.println(checksum);*/
              }
              // Turn on the Flag (Bật cờ thông báo: đã nhận xong dữ liệu từ xe gửi lên)
              // pRxData->RxFlag = 1;
            } else {
              // End byte sai -> Reset state machine
              pRxData->rxIndex = 0;
              pRxData->STATE = UART_WAIT;
              //Serial.println("Không thể tìm được END_BYTE");
            }
          } else {
            pRxData->rxIndex = pRxData->rxIndex + 1;  // Tăng index
          }
          break;
        }
    }
  }
}

/******************************************************************************/
// Hàm gửi lệnh xuống ESP32 Slave qua ESP-NOW
void ESPNOW_Send_Slave(uint8_t *pData) {
  uint8_t dataLength = (pData[2] == 0x00) ? shortFrameLength : longFrameLength;
  esp_err_t result = esp_now_send(SlaveAddress, (uint8_t *)pData, dataLength);
  /*if (result == ESP_OK) {
    Serial.println("Dữ liệu gửi tới ESP32 Slave thành công!");
  } else {
    Serial.print("Lỗi không thể gửi dữ liệu tới ESP32 Slave! Error code: ");
    Serial.println(result);
  }*/
}

/******************************************************************************/
// Hàm callback khi nhận dữ liệu từ ESP-NOW
void ESPNOW_callback(const esp_now_recv_info_t *esp_now_info, const uint8_t *esp_data, int data_len) {
  // Kiểm tra danh tính nguồn gửi đến
  if (memcmp(esp_now_info->src_addr, SlaveAddress, MAC_LEN)) {
    return;
  // Xử lý frame
  if (data_len == DATA_FRAME_LENGTH) {
    // Data frame -> Gửi dữ liệu STM32 lên WinForm (VS)
    memcpy(&slaveDataRC, esp_data, DATA_FRAME_LENGTH);
    Serial.write((uint8_t *)&slaveDataRC, DATA_FRAME_LENGTH);
    /*Serial.print("x Angle = ");
    Serial.println(slaveDataRC.xAngle / 100.0);
    Serial.print("z Angle = ");
    Serial.println(slaveDataRC.zAngle / 100.0);
    Serial.print("x Angular Velocity = ");
    Serial.println(slaveDataRC.xAngularVelocity / 100.0);
    Serial.print("z Angular Velocity = ");
    Serial.println(slaveDataRC.zAngularVelocity / 100.0);
    Serial.print("x Coordinate = ");
    Serial.println(slaveDataRC.xCoordinate / 50.0);
    Serial.print("Left Wheel Torque = ");
    Serial.println(slaveDataRC.leftWheelTorque / 100.0);
    Serial.print("Right Wheel Torque = ");
    Serial.println(slaveDataRC.rightWheelTorque / 100.0);
    Serial.print("\n");*/
  }
  if (data_len == EMPTY_FRAME_LENGTH) {
    // Empty frame -> Không gửi
    if (esp_data[0] == UPLINK_START_BYTE && esp_data[1] == EMPTY_ID 
                                         && esp_data[2] == END_BYTE) {
      Serial.print("Master vừa nhận được frame báo trống dữ liệu");
    }
  }
}

/******************************************************************************/
// Hàm callback của requestData_Timer
void vTimerCallback(TimerHandle_t requestData_Timer) {
  requestTimer_Flag = 1;
}

/******************************************************************************/
// Hàm khởi tạo ESP-NOW
void InitESPNow(void) {
  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Đăng ký hàm callback nhận dữ liệu từ ESP-NOW
  esp_now_register_recv_cb((esp_now_recv_cb_t)ESPNOW_callback);
  // Cấu hình ESP-NOW
  memcpy(peerInfo.peer_addr, SlaveAddress, MAC_LEN);  // Gán địa chỉ thiết bị nhận (ESP32 Slave)
  peerInfo.channel = 0;                               // Sử dụng kênh Wifi mặc định 0
  peerInfo.encrypt = false;                           // Không mã hoá
  // Thêm thiết bị nhận
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    //Serial.println("ADD Fail");
    return;
  }
}

/******************************************************************************/
void setup() {
  // Khởi tạo trạng thái state machine
  UART_Rx.STATE = UART_WAIT;
  // Khởi tạo serial monitor
  Serial.begin(115200);
  // Thiết lập esp32 là một trạm thu phát wi-fi
  WiFi.mode(WIFI_STA);
  // Khởi tạo ESP-NOW
  InitESPNow();
  // Khởi tạo Timer yêu cầu Slave gửi dữ liệu STM32
  requestData_Timer = xTimerCreate(
    "Timer yêu cầu Slave gửi dữ liệu",
    REQUEST_DATA_PERIOD,
    pdTRUE,
    NULL,
    vTimerCallback);
  // Kích hoạt Timer
  xTimerStart(requestData_Timer, 0);
}

void loop() {
  // Yêu cầu Slave gửi dữ liệu STM32
  if (requestTimer_Flag == 1) {
    // Yêu cầu Slave gửi dữ liệu STM32
    ESPNOW_Send_Slave(frame_Request_STM32Data);
    requestTimer_Flag = 0;
  }

  // Đọc dữ liệu từ WinForm (VS)
  Read_VS_Data(&UART_Rx);
  if (g_UART_RxFlag == 1) {
    // Chuyển tiếp gói tin xuống Slave
    ESPNOW_Send_Slave(UART_Rx.rxBuff);
    g_UART_RxFlag = 0;
  }

  vTaskDelay(1);
}

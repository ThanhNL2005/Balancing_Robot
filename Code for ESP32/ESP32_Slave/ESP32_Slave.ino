#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>

/******************************************************************************/
// Khởi tạo UART0
HardwareSerial mySerial(0);
// Khởi tạo handle: UART Mutex
SemaphoreHandle_t UART_Mutex = NULL;
// Khởi tạo handle: ESP-NOW Queue
QueueHandle_t ESPNOW_Queue = NULL;
// Khởi tạo handle: ESP-NOW Task
TaskHandle_t ESPNOW_Task = NULL;

/******************************************************************************/
// Header: Giao tiếp với STM32 qua UART0
#define UPLINK_START_BYTE 0xAA
#define END_BYTE 0xED
#define rxLength 18
#define UART_READING 1
#define UART_WAIT 0
typedef struct
{
  byte rxBuff[rxLength];
  uint8_t rxIndex;
  uint8_t rxByte;
  uint8_t STATE;
} UART_RxData_t;  // Struct của biến thực hiện nhận dữ liệu UART0
UART_RxData_t UART_Rx_Buffer;
UART_RxData_t UART_Rx;     
uint8_t gRxSTM32Flag = 0;  // Cờ báo nhận thành công dữ liệu STM32

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
} STM32_Data_t;  // Cấu trúc dữ liệu của STM32 Data
STM32_Data_t stm32DataRC;
#pragma pack()

/******************************************************************************/
// Header: Giao tiếp với ESP32 Master qua ESP-NOW
#define max_frame_len 12
#define longFrameLength 12
#define shortFrameLength 9
#define MAC_LEN 6
#define STM32DATA_ID 0x04  // Mã từ ESP32 Master yêu cầu gửi dữ liệu STM32 lên
#define EMPTY_ID 0xEE      // Mã từ ESP32 Slave báo trống dữ liệu

#pragma pack(1)
struct
{
  uint8_t firstCheck = UPLINK_START_BYTE;
  uint8_t ID_Number = EMPTY_ID;
  uint8_t lastCheck = END_BYTE;
} Empty_Frame; // Frame gửi cho Master khi không đọc được dữ liệu STM32
#pragma pack()

uint8_t master_address[] = { 0xA0, 0xDD, 0x6C, 0x02, 0xCF, 0x94 };  // Địa chỉ MAC của thiết bị nhận (ESP32 Master)
esp_now_peer_info_t peerInfo;                                       // Biến lưu thông tin thiết bị nhận

/******************************************************************************/
// Hàm tính mã Modbus CRC16
uint16_t Modbus_CRC16(uint8_t *data, uint16_t Length) {             // Số phần tử trong data cần tính mã CRC
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = 0; pos < Length; pos++) {
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
void Read_STM32_Data(UART_RxData_t *pRxData) {
  if (pRxData == NULL) {
    return;
  }
  while (mySerial.available() > 0) {
    pRxData->rxByte = mySerial.read();
    switch (pRxData->STATE) {
      case UART_WAIT:
        {
          pRxData->rxBuff[0] = pRxData->rxByte;  // Dò start byte để lấy điểm bắt đầu đọc
          if (pRxData->rxBuff[0] == UPLINK_START_BYTE) {
            pRxData->STATE = UART_READING;  // Chuyển sang trạng thái đọc
            // Serial.println("Đã chuyển sang trạng thái Reading");
            pRxData->rxIndex = 1;
          } else {
            // Byte đầu tiên không phải UPLINK_START_BYTE
            //Serial.println("Không thể chuyển sang trạng thái Reading");
          }
          break;
        }
      case UART_READING:
        {
          pRxData->rxBuff[pRxData->rxIndex] = pRxData->rxByte;  // Lưu từng byte
          // Kiểm tra end byte
          if (pRxData->rxIndex == rxLength - 1) {
            if (pRxData->rxBuff[pRxData->rxIndex] == END_BYTE) {
              /* Serial.println("Frame đọc được là: ");
              for (int i = 0; i <= pRxData->rxIndex; i++) {
                 Serial.printf("%02X ", pRxData->rxBuff[i]);
              }
              Serial.println("\n");*/
              // Kiểm tra Checksum
              uint16_t CRC16 = Modbus_CRC16(pRxData->rxBuff, rxLength - 3);
              uint8_t Low_Byte_CRC16 = (uint8_t)(CRC16 & 0xFF);
              uint8_t High_Byte_CRC16 = (uint8_t)((CRC16 >> 8) & 0xFF);
              if (Low_Byte_CRC16 == pRxData->rxBuff[rxLength - 3] && High_Byte_CRC16 == pRxData->rxBuff[rxLength - 2]) {
                // Checksum đúng -> Bật cờ thông báo và reset state machine
                if (xSemaphoreTake(UART_Mutex, portMAX_DELAY) == pdTRUE) {
                  memcpy(UART_Rx.rxBuff, pRxData->rxBuff, rxLength);  // Lưu vào UART_Rx
                  gRxSTM32Flag = 1;                                   // Dựng cờ báo đọc dữ liệu từ STM32 thành công
                  xSemaphoreGive(UART_Mutex);
                }
                pRxData->rxIndex = 0;
                pRxData->STATE = UART_WAIT;
                // Serial.println("Checksum đúng. Hoàn thành nhận dữ liệu.");
              } else {
                // Checksum sai -> Reset state machine
                pRxData->rxIndex = 0;
                pRxData->STATE = UART_WAIT;
                //Serial.println("Checksum sai. Frame bị loại bỏ.");
              }
            } else {
              // Frame bị mất END_BYTE -> Reset state machine
              pRxData->rxIndex = 0;
              pRxData->STATE = UART_WAIT;
              // Serial.println("Không thể tìm được END_BYTE");
            }
          } else {
            pRxData->rxIndex = pRxData->rxIndex + 1;  // Tăng index của mảng lưu
          }
          break;
        }
    }
  }
}

/******************************************************************************/
// Hàm callback khi nhận dữ liệu qua ESP-NOW
void ESPNOW_callback(const esp_now_recv_info_t *esp_now_info, const uint8_t *esp_data, int data_len) {
  // Kiểm tra danh tính nguồn gửi đến
  if (memcmp(esp_now_info->src_addr, master_address, MAC_LEN)) {
    return;
  }
  // Lưu dữ liệu vào ESPNOW Queue
  if (data_len == shortFrameLength) {
    uint8_t buff[max_frame_len] = { 0 };
    memcpy(buff, esp_data, data_len);
    xQueueSend(ESPNOW_Queue, buff, 0);
  } else if (data_len == longFrameLength) {
    xQueueSend(ESPNOW_Queue, esp_data, 0);
  }
}

/******************************************************************************/
// Hàm xử lý dữ liệu nhận từ ESP32 Master
void Process_Master_Data(void *pvParameters) {
  uint8_t ESP_Buffer[max_frame_len];
  while (1) {
    if (xQueueReceive(ESPNOW_Queue, ESP_Buffer, portMAX_DELAY) == pdTRUE) {
      if (ESP_Buffer[1] == STM32DATA_ID) {
        // Thực hiện lệnh: Gửi dữ liệu STM32 định kỳ lên cho Master
        STM32Data_To_Master(&UART_Rx);
      } else {
        // Thục hiện lệnh: Chuyển tiếp gói tin từ WinForm (VS) xuống STM32
        MasterData_To_STM32(ESP_Buffer);
      }
    }
  }
  /*uxTaskGetStackHighWaterMark(NULL)*/  // Kiểm tra dung lượng stack thực tế mà Task sử dụng
}

/******************************************************************************/
// Hàm gửi dữ liệu STM32 lên Master
void STM32Data_To_Master(UART_RxData_t *pRxData) {
  if (xSemaphoreTake(UART_Mutex, portMAX_DELAY) == pdTRUE) {
    if (gRxSTM32Flag) {
      memcpy(&stm32DataRC, pRxData->rxBuff, sizeof(STM32_Data_t) - 1); // Bỏ lại 2 byte CRC
      stm32DataRC.endCheckValue = END_BYTE;
      // Serial.println("Data received successfully from STM32!");
      // Serial.print("x Angle = ");
      // Serial.println(stm32DataRC.xAngle);
      // Serial.print("z Angle = ");
      // Serial.println(stm32DataRC.zAngle);
      // Serial.print("x Angular Velocity = ");
      // Serial.println(stm32DataRC.xAngularVelocity);
      // Serial.print("z Angular Velocity = ");
      // Serial.println(stm32DataRC.zAngularVelocity);
      // Serial.print("x Coordinate = ");
      // Serial.println(stm32DataRC.xCoordinate);
      // Serial.print("Left Wheel Torque = ");
      // Serial.println(stm32DataRC.leftWheelTorque);
      // Serial.print("Right Wheel Torque = ");
      // Serial.println(stm32DataRC.rightWheelTorque);
      gRxSTM32Flag = 0;
      xSemaphoreGive(UART_Mutex);
      // Gửi dữ liệu STM32 cho Master
      esp_err_t resultA = esp_now_send(master_address, (uint8_t *)&stm32DataRC, sizeof(stm32DataRC));
      // if (resultA == ESP_OK) {
      //   Serial.println("Dữ liệu gửi sang ESP32 Master thành công!");
      // }
      // else
      // {
      //   Serial.print("Không thể gửi dữ liệu sang ESP32 Master! Error code: ");
      //   Serial.println(resultA);
      // }
    } else {
      xSemaphoreGive(UART_Mutex);
      // Gửi frame báo trống dữ liệu cho Master
      esp_err_t resultB = esp_now_send(master_address, (uint8_t *)&Empty_Frame, sizeof(Empty_Frame));
    }
  }
}

/******************************************************************************/
// Hàm gửi dữ liệu từ Master xuống STM32
void MasterData_To_STM32(uint8_t *buff) {
  uint8_t buff_len = buff[2] == 0x00 ? shortFrameLength : longFrameLength;
  mySerial.write(buff, buff_len);
}

/******************************************************************************/
// Hàm khởi tạo ESP-NOW
void InitESPNow(void) {
  if (esp_now_init() != ESP_OK) {
    // Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Đăng ký hàm callback trong ESPNOW
  esp_now_register_recv_cb((esp_now_recv_cb_t)ESPNOW_callback);
  // Cấu hình ESP-NOW
  memcpy(peerInfo.peer_addr, master_address, MAC_LEN);  // Gán địa chỉ thiết bị nhận
  peerInfo.channel = 0;                                 // Sử dụng kênh Wifi mặc định 0
  peerInfo.encrypt = false;                             // Không mã hoá
  // Thêm thiết bị nhận (ESP32 Master)
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    // Serial.println("Adding peer Failed");
    return;
  }
}

/******************************************************************************/
void setup() {
  // Khởi tạo trạng thái state machine
  UART_Rx_Buffer.STATE = UART_WAIT;
  // Khởi tạo serial monitor
  Serial.begin(115200);
  // Thiết lập esp32 là một trạm thu phát wi-fi
  WiFi.mode(WIFI_STA);
  // Khởi tạo phần cứng UART0
  mySerial.begin(115200, SERIAL_8N1, 3, 1);
  // Khởi tạo UART Mutex
  UART_Mutex = xSemaphoreCreateMutex();
  // Khởi tạo ESP-NOW Queue
  ESPNOW_Queue = xQueueCreate(32, max_frame_len);
  // Khởi tạo Task xử lý dữ liệu từ Master
  xTaskCreatePinnedToCore(Process_Master_Data,
                          "Xu ly du lieu Master",
                          4096,  // Dung lượng stack 4096 bytes
                          NULL,
                          5,     // Độ ưu tiên 5
                          &ESPNOW_Task,
                          1);    // Chạy trên Core 1
  // Khởi tạo ESP-NOW
  InitESPNow();
}

void loop() {
  Read_STM32_Data(&UART_Rx_Buffer);
  vTaskDelay(1);
}
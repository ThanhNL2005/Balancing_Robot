#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include "driver/uart.h"
#include <HardwareSerial.h>

/******************************************************************************/
HardwareSerial mySerial(2); // USART2
/******************************************************************************/
// Định nghĩa frame dữ liệu nhận từ Winfrom
#define START_BYTE 0xBB
#define END_BYTE 0x66
#define longFrameLength 11
#define shortFrameLength 8

#define USART_READING 1
#define USART_WAIT 0
/**********************************************************/
// Địa chỉ MAC broadcast (gửi đến tất cả các thiết bị)
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Slave: {0x78, 0xE3, 0x6D, 0xDD, 0xD9, 0x48}
uint8_t broadcastAddress[] = {0x78, 0xE3, 0x6D, 0xDD, 0xD9, 0x48};
esp_now_peer_info_t peerInfo; // Bien luu thong tin thiet bi nhan
uint8_t espNow_RxFlag;  //Receive interrupt flag
/******************************************************************************/
typedef struct
{
  byte rxBuff[100];
  byte txBuff[100];
  uint8_t RxFlag;
  uint8_t frameStart, frameEnd, frameLength, groupID, senderAddr, receiverAddr, messType, dataLength;
  uint16_t messCount, rxPointer;
  uint8_t rxByte;
  uint8_t STATE;
} usartData;
usartData usart_Tx, usart_Rx;
uint8_t usartRxLength_expected = shortFrameLength;
/****************** Kiem tra checksum doi voi frame du lieu nhan tu STM32 ************************/
uint16_t Modbus_CRC16(uint8_t* data, uint16_t frameLength, uint16_t pos_start, uint16_t pos_end)
{
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = pos_start; pos <= pos_end; pos++){
    crc ^= (uint16_t)data[pos];
    for (uint8_t i=0; i<8; i++) {
      if ((crc & 0x0001)== 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else {
        crc >>= 1;
      }
    }
  }
  return crc;
}
uint8_t checksum = 0;
/****************************************************************************************/
// Cấu trúc dữ liệu nhận từ ESP32 Slave
#pragma pack(1)
typedef struct
{
  uint8_t firstCheck;
  int16_t xAngle;
  int16_t zAngle;
  int16_t xAngularVelocity;
  int16_t zAngularVelocity;
  int16_t xCoordinate;
  int16_t leftWheelTorque;
  int16_t rightWheelTorque; // Tần số lấy mẫu của vi xử lý trên STM32
  uint8_t lastCheck;
} Slave_Data_Received;
#pragma pack()
Slave_Data_Received slaveDataRC;
/******************************************************************************/
void usart_DataRead(usartData *pRxData)
{
  int i = 0;
  while (Serial.available() > 0)  // Check uart data (Kiểm tra có byte gửi qua UART không)
  {
    pRxData->rxByte = Serial.read();
    switch (pRxData->STATE)
    {
      case USART_WAIT:
      {
        pRxData->rxBuff[0] = pRxData->rxByte;  // Read first byte
        if (pRxData->rxBuff[0] == START_BYTE)
        {
          pRxData->STATE = USART_READING;           // Switch to Reading state
          Serial.println("Đã chuyển sang trạng thái Reading");
          usartRxLength_expected = shortFrameLength;
          pRxData->rxPointer = 1;
        }
        else  // Fail (Byte đầu tiên không phải START_BYTE)
        {
          Serial.println("Không thể chuyển sang trạng thái Reading");
          pRxData->rxPointer = 0;         //Reset pointer
        }
        break;
      }
      case USART_READING:
      {
        pRxData->rxBuff[pRxData->rxPointer] = pRxData->rxByte;  //Read data
        if (pRxData->rxPointer == 3 && pRxData->rxBuff[2] == 0x01)
          usartRxLength_expected = longFrameLength;
        if (pRxData->rxPointer == usartRxLength_expected - 1)
        {
          if (pRxData->rxBuff[pRxData->rxPointer] == END_BYTE)
          {
            // Save data
            Serial.println("Frame đọc được là: ");
            for (int i = 0; i <= pRxData->rxPointer; i++) 
            {
              Serial.printf("%02X ", pRxData->rxBuff[i]);
            }
            Serial.println("\n");

            // Kiểm tra Checksum
            checksum = Modbus_CRC16(pRxData->rxBuff, 100, 0, usartRxLength_expected-3);
            if (checksum == pRxData->rxBuff[usartRxLength_expected-2])
            {
              // Checksum đúng. Bật cờ thông báo: đã nhận xong dữ liệu từ xe gửi lên
              Serial.print("Vị trí của byte checksum: ");
              Serial.println(usartRxLength_expected-2); // Frame 8 bytes: 6, frame 11 bytes: 9
              Serial.println("Checksum đúng. Hoàn thành nhận dữ liệu.");
              Serial.print("Checksum nhận: ");
              Serial.println(pRxData->rxBuff[usartRxLength_expected-2]);
              Serial.print("Checksum tính toán: ");
              Serial.println(checksum);
              pRxData->RxFlag = 1;
            }
            else
            {
              // Checksum sai. Reset toàn bộ frame, đặt cờ RxFlag về bằng 0, trở lại trạng thái chờ
              Serial.print("Vị trí của byte checksum: ");
              Serial.println(usartRxLength_expected-2); // Frame 8 bytes: 6, frame 11 bytes: 9
              Serial.println("Checksum sai. Frame sẽ bị loại bỏ.");
              Serial.print("Checksum nhận: ");
              Serial.println(pRxData->rxBuff[usartRxLength_expected-2]);
              Serial.print("Checksum tính toán: ");
              Serial.println(checksum);
              
              pRxData->rxPointer = 0;
              pRxData->RxFlag = 0;
              for (i = 0; i <= pRxData->rxPointer; i++)
              {
                pRxData->rxBuff[i] = 0;
              }
            } 
            // Turn on the Flag (Bật cờ thông báo: đã nhận xong dữ liệu từ xe gửi lên)
            // pRxData->RxFlag = 1;
          }
          else  //Fail (Frame bị sai mất END_BYTE) -> Reset buffer, pointer and move to WAIT state
          {
            Serial.println("Không thể tìm được END_BYTE");
            pRxData->rxPointer = 0;
            pRxData->RxFlag = 0;
            for (i = 0; i <= pRxData->rxPointer; i++)
            {
              pRxData->rxBuff[i] = 0;
            }
          }
          pRxData->STATE = USART_WAIT;
        }
        else 
        {
          pRxData->rxPointer = pRxData->rxPointer + 1;    //Increse the pointer
          if (pRxData->rxPointer > usartRxLength_expected - 1)            //Fail
          {
            pRxData->rxPointer = 0;
            pRxData->RxFlag = 0;
            for (i = 0; i < usartRxLength_expected; i++)
            {
              pRxData->rxBuff[i] = 0;
            }
            pRxData->STATE = USART_WAIT;
          }
        }
        break;
      }
    }
  }
  // Process receive data (Nếu có byte dữ liệu đến, xử lý byte đó)
}
/******************************************************************************/
void espNowSlaveSend(usartData *pRxData, uint8_t dataLength)
{
  // Gui goi du lieu nhan sang ESP32 slave
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)pRxData, dataLength);
  if (result == ESP_OK) {
    Serial.println("Dữ liệu gửi tới ESP32 Slave thành công!");
  }
  else
  {
    Serial.print("Lỗi không thể gửi dữ liệu tới ESP32 Slave! Error code: ");
    Serial.println(result);
  }
}
/******************************************************************************/
// Ham callback khi nhan du lieu tu Slave
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(Slave_Data_Received))
  {
    memcpy(&slaveDataRC, incomingData, sizeof(Slave_Data_Received));
    Serial.print("First check value = ");
    Serial.println(slaveDataRC.firstCheck);
    Serial.print("x Angle = ");
    Serial.println(slaveDataRC.xAngle/100.0);
    Serial.print("z Angle = ");
    Serial.println(slaveDataRC.zAngle/100.0);
    Serial.print("x Angular Velocity = ");
    Serial.println(slaveDataRC.xAngularVelocity/100.0);
    Serial.print("z Angular Velocity = ");
    Serial.println(slaveDataRC.zAngularVelocity/100.0);
    Serial.print("x Coordinate = ");
    Serial.println(slaveDataRC.xCoordinate/50.0);
    Serial.print("Left Wheel Torque = ");
    Serial.println(slaveDataRC.leftWheelTorque/100.0);
    Serial.print("Sampling Frequency = ");
    Serial.println(slaveDataRC.rightWheelTorque/100.0);
    Serial.print("Right Wheel Torque = ");
    Serial.println(slaveDataRC.lastCheck);
    Serial.print("\n");
    espNow_RxFlag = 1;
    Serial.write((uint8_t *)&slaveDataRC, sizeof(Slave_Data_Received));
  }
  else
  {
    Serial.println("Data size mismatch!");
    //espNow_RxFlag = 0;
  }
}
/******************************************************************************/
void InitESPNow(void)
{
  espNow_RxFlag = 0;

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Dang ki ham callback gui du lieu tu Slave va nhan du lieu tu Master
  esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
  //esp_now_register_send_cb(OnDataSend);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6); // Gan dia chi thiet bi nhan
  peerInfo.channel = 0;                        // Su dung kenh Wifi mac dinh 0
  peerInfo.encrypt = false;                    // Khong ma hoa

  // Them thiet bi nhan
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("ADD Fail");
    return;
  }
}
/******************************************************************************/
void setup() {
  // Init Serial Monitor (Khởi tạo serial monitor)
  Serial.begin(115200);
  usart_Rx.STATE = USART_WAIT;
  usart_Rx.RxFlag = 0;
  // Set device as a Wi-Fi Station (Thiết lập esp32 là một trạm thu phát wi-fi)
  WiFi.mode(WIFI_STA);
  // Initialize ESP-NOW
  InitESPNow();

  //mySerial.begin(115200, SERIAL_8N1, 16, 17);
}

void loop() {
  // Get USART data
  if (usart_Rx.RxFlag == 0)
  {
    usart_DataRead(&usart_Rx);
  }
  else
  {
    // Xử lý dữ liệu USART đã nhận đầy đủ
    espNowSlaveSend(&usart_Rx, usartRxLength_expected);
    //Clear Rx Flag
    usart_Rx.RxFlag = 0;
  }
}

#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>

/**********************************************************/
// Khởi tạo USART0
HardwareSerial mySerial(0); // USART0
/**********************************************************/
// Định nghĩa frame mẫu dữ liệu nhận từ STM32 
#define START_BYTE 0xAA
#define END_BYTE 0x55
#define CHECKSUM 0xFF
#define ID 0x01
#define LENGTH 0x08
#define SAMPLING_PERIOD_LowByte 0x01
#define SAMPLING_PERIOD_HighByte 0x00
#define rxLength 19 

#define USART_READING 1
#define USART_WAIT 0
/**********************************************************/
// Định nghĩa frame nhận qua ESP-Now từ ESP32 Master
#define StartByteFromMaster 0xBB
#define EndByteFromMaster 0x66
#define longFrameLength 11
#define shortFrameLength 8
uint8_t ESPBuffer[longFrameLength];
uint8_t bufferIndex = 0;
bool receivingFrame = false;
uint8_t FrameSize_expected = shortFrameLength;
/**********************************************************/
// Địa chỉ MAC của thiết bị nhận
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //Master: {0xA0, 0xDD, 0x6C, 0x02, 0xCF, 0x94}
uint8_t broadcastAddress[] = {0xA0, 0xDD, 0x6C, 0x02, 0xCF, 0x94};
esp_now_peer_info_t peerInfo; // Bien luu thong tin thiet bi nhan
uint8_t espNow_RxFlag;  //Receive interrupt flag
/*******************************************************************************************/
// Cấu trúc dữ liệu nhận từ STM32
#pragma pack(1)
typedef struct
{
  uint8_t firstCheck = 24;
  int16_t xAngle;
  int16_t zAngle;
  int16_t xAngularVelocity;
  int16_t zAngularVelocity;
  int16_t xCoordinate;
  int16_t leftWheelTorque;
  int16_t rightWheelTorque; // Tần số lấy mẫu của vi xử lý trên STM32
  uint8_t lastCheck = 25;
} STM32_Data_Received;
#pragma pack()
STM32_Data_Received stm32DataRC; 
/*******************************************************************************************/
typedef struct
{
  byte rxBuff[rxLength];
  byte txBuff[100];
  uint8_t RxFlag;
  uint8_t frameStart, frameEnd, frameLength, groupID, senderAddr, receiverAddr, messType, dataLength;
  uint16_t messCount, rxPointer;
  uint8_t rxByte;
  uint8_t STATE;
} usartData;
usartData usart_Tx, usart_Rx;
/****************** Kiem tra checksum doi voi frame du lieu nhan tu STM32 ************************/
uint16_t Modbus_CRC16(uint8_t* data, uint16_t frameLength, uint16_t pos_start, uint16_t pos_end)
{
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = pos_start; pos <= pos_end; pos++){
    crc ^= (uint16_t)data[pos];
    for (uint8_t i=0; i<8; i++) {
      if (crc & 0x0001) {
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
void usart_DataRead(usartData *pRxData)  // Save received data (Lưu dữ liệu nhận được)
{
  int i = 0;
  while (mySerial.available() > 0)  // Check uart data (Kiểm tra có byte gửi qua UART không)
  {
    pRxData->rxByte = mySerial.read();
    switch (pRxData->STATE)
    {
      case USART_WAIT:
      {
        pRxData->rxBuff[0] = pRxData->rxByte;  // Read first byte
        if (pRxData->rxBuff[0] == START_BYTE)
        {
          pRxData->STATE = USART_READING;           // Switch to Reading state
          // Serial.println("Đã chuyển sang trạng thái Reading");
          pRxData->rxPointer = 1;
        }
        else  // Fail (Byte đầu tiên không phải START_BYTE)
        {
          //Serial.println("Không thể chuyển sang trạng thái Reading");
          pRxData->rxPointer = 0;         //Reset pointer
        }
        break;
      }
      case USART_READING:
      {
        pRxData->rxBuff[pRxData->rxPointer] = pRxData->rxByte;  //Read data
        if (pRxData->rxPointer == rxLength - 1)
        {
          if (pRxData->rxBuff[pRxData->rxPointer] == END_BYTE)
          {
            // Save data
            // Serial.println("Frame đọc được là: ");
            for (int i = 0; i <= pRxData->rxPointer; i++) 
            {
              // Serial.printf("%02X ", pRxData->rxBuff[i]);
            }
            // Serial.println("\n");
            // Checksum
            // checksum = Modbus_CRC16(pRxData->rxBuff, rxLength, 0, rxLength-3);
            // if (checksum == pRxData->rxBuff[rxLength-2])
            // {
            //   // Checksum đúng. Bật cờ thông báo: đã nhận xong dữ liệu từ xe gửi lên
            //   // Serial.println("Checksum đúng. Hoàn thành nhận dữ liệu.");
            //   pRxData->RxFlag = 1;
            // }
            // else
            // {
            //   // Checksum sai. Reset toàn bộ frame, đặt cờ RxFlag về bằng 0, trở lại trạng thái chờ
            //   // Serial.println("Checksum sai. Frame sẽ bị loại bỏ.");
            //   pRxData->rxPointer = 0;
            //   pRxData->RxFlag = 0;
            //   for (i = 0; i <= pRxData->rxPointer; i++)
            //   {
            //     pRxData->rxBuff[i] = 0;
            //   }
            // } 
            pRxData->RxFlag = 1;
          }
          else  //Fail (Frame bị sai mất END_BYTE) -> Reset buffer, pointer and move to WAIT state
          {
            // Serial.println("Không thể tìm được END_BYTE");
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
          if (pRxData->rxPointer > rxLength-1)            //Fail
          {
            pRxData->rxPointer = 0;
            pRxData->RxFlag = 0;
            for (i = 0; i < rxLength; i++)
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
/*******************************************************************************************/
void espNowMasterSend(usartData *pRxData)
{
  stm32DataRC.xAngle = (int16_t)(pRxData->rxBuff[3] | (pRxData->rxBuff[4] << 8));   
  stm32DataRC.zAngle = (int16_t)(pRxData->rxBuff[5] | (pRxData->rxBuff[6] << 8));   
  stm32DataRC.xAngularVelocity = (int16_t)(pRxData->rxBuff[7] | (pRxData->rxBuff[8] << 8));   
  stm32DataRC.zAngularVelocity = (int16_t)(pRxData->rxBuff[9] | (pRxData->rxBuff[10] << 8));   
  stm32DataRC.xCoordinate = (int16_t)(pRxData->rxBuff[11] | (pRxData->rxBuff[12] << 8));   
  stm32DataRC.leftWheelTorque = (int16_t)(pRxData->rxBuff[13] | (pRxData->rxBuff[14] << 8));   
  stm32DataRC.rightWheelTorque = (int16_t)(pRxData->rxBuff[15] | (pRxData->rxBuff[16] << 8));

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

  // Gui goi du lieu nhan tu STM32 sang ESP32 Master
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&stm32DataRC, sizeof(stm32DataRC));
  // if (result == ESP_OK) {
  //   Serial.println("Dữ liệu gửi sang ESP32 Master thành công!");
  // }
  // else
  // {
  //   Serial.print("Không thể gửi dữ liệu sang ESP32 Master! Error code: ");
  //   Serial.println(result);
  // }
}
/*******************************************************************************************/
// Ham callback khi nhan du lieu tu Master
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  for (int i = 0; i < FrameSize_expected; i++)
  {
    uint8_t byte = incomingData[i];
    if (!receivingFrame) {
      if (byte == StartByteFromMaster) {
        receivingFrame = true;
        bufferIndex = 0; 
        FrameSize_expected = shortFrameLength;
        ESPBuffer[bufferIndex++] = byte;
      }
      else {
        // Serial.println("Start byte từ frame của Master gửi bị sai!");
        espNow_RxFlag = 0;
      }
    }
    else {
      if (bufferIndex == 3 && ESPBuffer[2] == 0x01) {
        FrameSize_expected = longFrameLength;
      }
      ESPBuffer[bufferIndex++] = byte;
      if (bufferIndex == FrameSize_expected) {
        // Đủ byte, kiểm tra EndByte
        if (ESPBuffer[FrameSize_expected - 1] == EndByteFromMaster) {
          // Serial.println("Đã nhận EndByte");
          espNow_RxFlag = 1;
        }
        else {
          // Serial.println("Frame không hợp lệ (thiếu EndByte)");
          espNow_RxFlag = 0;
        }
      }
    }
  }
  // Reset để nhận frame tiếp theo
  receivingFrame = false;
  bufferIndex = 0;
}
/******************************************************************************/
// Giải mã buffer từ Master
void DecodeESPbuffer(uint8_t* buff, uint8_t buffSize){
  if (buff[2] == 0x00){
    int16_t temp = (int16_t)((buff[5] << 8) | buff[4]);
    // Serial.println("Frame ngắn: loại dữ liệu, thứ tự cùng giá trị nhận được:");
    // Serial.printf("%d, %d, %d\n", buff[1], buff[3], temp);
    // Serial.println("Received full frame:");
    for (int i = 0; i < buffSize; i++) {
      // Serial.printf("%02X ", buff[i]);
    }
    // Serial.println();
  }
  else if (buff[2] == 0x01){
    int16_t temp0 = (int16_t)((buff[4] << 8) | buff[3]);
    int16_t temp1 = (int16_t)((buff[6] << 8) | buff[5]);
    int16_t temp2 = (int16_t)((buff[8] << 8) | buff[7]);
    // Serial.println("Frame dài: loại dữ liệu và các giá trị nhận được:");
    // Serial.printf("%d, %d, %d, %d\n", buff[1], temp0, temp1, temp2);
    // Serial.println("Received full frame:");
    for (int i = 0; i < buffSize; i++) {
      // Serial.printf("%02X ", buff[i]);
    }
    // Serial.println();
  }
} 
/******************************************************************************/
// Code gửi dữ liệu xuống STM32 (không được xoá đi)
void SendToSTM32(uint8_t* buff, uint8_t buffSize)
{
  mySerial.write(buff, buffSize);
}
/******************************************************************************/
void InitESPNow(void)
{
  espNow_RxFlag = 0;

  if (esp_now_init() != ESP_OK)
  {
    // Serial.println("Error initializing ESP-NOW");
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
    // Serial.println("Adding peer Failed");
    return;
  }
}
/*******************************************************************************************/
void setup() {
  // Init Serial Monitor (Khởi tạo serial monitor)
  Serial.begin(115200);
  usart_Rx.STATE = USART_WAIT;
  usart_Rx.RxFlag = 0;
  // Set device as a Wi-Fi Station (Thiết lập esp32 là một trạm thu phát wi-fi)
  WiFi.mode(WIFI_STA);
  // Initialize ESP-NOW
  InitESPNow();
  // Initialize hardware USART
  mySerial.begin(115200, SERIAL_8N1, 3, 1);
}

void loop() {
  // Nhận dữ liệu UART từ STM32 gửi lên
  if (usart_Rx.RxFlag == 0)
  {
    usart_DataRead(&usart_Rx);
  }
  else
  {
    // Xử lý dữ liệu USART đã nhận đầy đủ
    espNowMasterSend(&usart_Rx);
    //Clear Rx Flag
    usart_Rx.RxFlag = 0;
  }

  // Nhận dữ liệu ESP-NOW từ ESP32 Master gửi đến
  if (espNow_RxFlag == 1)
  {
    DecodeESPbuffer(ESPBuffer, FrameSize_expected);
    SendToSTM32(ESPBuffer, FrameSize_expected);
    espNow_RxFlag = 0;
  }
}

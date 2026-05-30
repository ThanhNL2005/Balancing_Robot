#include "main.h"

#define RX_SIZE 100 
#define COMBINE_2BYTES(high, low) ((int16_t)(((int16_t)(high) << 8) | (low)))

extern UART_HandleTypeDef huart8;;
extern DMA_HandleTypeDef hdma_uart8_tx;
extern DMA_HandleTypeDef hdma_uart8_rx;
extern HWT901 HWT901_data;
extern uint8_t RxData[RX_SIZE];  // store data process
extern uint8_t rxBuffer_HWT901[RX_SIZE] ; // store ram data
uint8_t cmd_request[] = {0x50, 0x03, 0x00, 0x34, 0x00, 0x0C, 0x09, 0x80};
uint8_t size_HWT901 = 0;

void HWT901_request(void)
{
	  HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_SET);
		HAL_UART_Transmit_DMA(&huart8, cmd_request, sizeof(cmd_request));
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_RESET);

}
void calis_angle(void)
{
    uint8_t cmd_request[] = {0x50, 0x06, 0x00, 0x08, 0x00, 0x04, 0x9A, 0xC5};
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_SET);		HAL_UART_Transmit(&huart8, cmd_request, sizeof(cmd_request),100);
		HAL_Delay(10);
}
uint16_t Modbus_CRC16(uint8_t *data, uint16_t length)
{
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

void process_HWT901(HWT901 *hwt901,uint8_t *data)
{
 uint16_t crc = Modbus_CRC16(data, 27);
	if (data[27] == (uint8_t)(crc & 0xFF) && data[28] == (uint8_t)((crc >> 8) & 0xFF))
	{
// acceleration
hwt901->acceleration.x = ((float)COMBINE_2BYTES(data[3], data[4]) / 32768.0f) * 16.0f;
hwt901->acceleration.y = ((float)COMBINE_2BYTES(data[5], data[6]) / 32768.0f) * 16.0f;
hwt901->acceleration.z = ((float)COMBINE_2BYTES(data[7], data[8]) / 32768.0f) * 16.0f;
//angular_velocity
hwt901->angular_velocity.x = ((float)COMBINE_2BYTES(data[9], data[10]) / 32768.0f) * 2000.0f;
hwt901->angular_velocity.y = ((float)COMBINE_2BYTES(data[11], data[12]) / 32768.0f) * 2000.0f;
hwt901->angular_velocity.z = ((float)COMBINE_2BYTES(data[13], data[14]) / 32768.0f) * 2000.0f;
//angle
hwt901->angle.x = ((float)COMBINE_2BYTES(data[21], data[22]) / 32768.0f) * 180.0f;
hwt901->angle.y = ((float)COMBINE_2BYTES(data[23], data[24]) / 32768.0f) * 180.0f;
hwt901->angle.z = ((float)COMBINE_2BYTES(data[25], data[26]) / 32768.0f) * 180.0f;
	}
}

//void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
//{
//     HAL_UARTEx_ReceiveToIdle_DMA(&huart8, rxBuffer, RX_SIZE);
//     __HAL_DMA_DISABLE_IT(&hdma_uart8_rx, DMA_IT_HT);
//     size_HWT901 = Size;
//}
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//      for (int i=0; i<400;i++);
//		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_RESET);
//		HAL_UARTEx_ReceiveToIdle_DMA(&huart8, rxBuffer,29);
//		__HAL_DMA_DISABLE_IT(&hdma_uart8_rx, DMA_IT_HT);
//}


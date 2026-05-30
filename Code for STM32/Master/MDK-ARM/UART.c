#include "main.h"
#include <stdint.h>
#include <string.h>

//HWT906
#define  RX_SIZE 33
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart8;;
extern DMA_HandleTypeDef hdma_uart8_tx;
extern DMA_HandleTypeDef hdma_uart8_rx;
extern HWT906 HWT906_data;
extern HWT901 HWT901_data;
extern uint8_t RxData[RX_SIZE];  // store data process
extern uint8_t rxBuffer[RX_SIZE] ; // store ram data
extern uint8_t rxBuffer_HWT901[RX_SIZE] ; // store ram data
extern DMA_HandleTypeDef hdma_uart7_rx;
int count=0;
//ESP32
extern UART_HandleTypeDef huart6;
extern ESP32 esp32;
extern uint8_t frame[19] ;
extern int start;
extern float angle_psi_1;
extern uint8_t RS485_frame[12];


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART7) {
        memcpy(RxData, rxBuffer, RX_SIZE);			 
			  rearrange_frame(rxBuffer,RxData);
        process_frame(&HWT906_data,RxData);
        HAL_UART_Receive_DMA(&huart7, rxBuffer, RX_SIZE);
//			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_4);
    }
		if (huart->Instance == USART6 ){
		esp32.rec_mode=1;
		HAL_UART_Receive_DMA(&huart6, frame,11);
		}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
		if (huart->Instance == USART6 ){
		esp32.rec_mode=1;
		HAL_UARTEx_ReceiveToIdle_DMA(&huart6, frame,15);
		}
		if (huart->Instance == UART8 ){
		process_HWT901(&HWT901_data,rxBuffer_HWT901);
			count++;
		if(start==0 && count == 20){
			 angle_psi_1=HWT901_data.angle.z*3.1415926f/180.0f;
			 start=1;
			}
		}
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
//      for (int i=0; i<400;i++);
//		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_2,GPIO_PIN_RESET);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart8, rxBuffer_HWT901,29);
		__HAL_DMA_DISABLE_IT(&hdma_uart8_rx, DMA_IT_HT);
}


#include "main.h"
#include "math.h"
#include <string.h>

/*uart data*/
extern UART_HandleTypeDef huart4; //RS485
extern UART_HandleTypeDef huart5; //For NEXTION HMI
extern usartData usart4_Tx, usart4_Rx; 		//RS485
extern usartData usart5_Tx, usart5_Rx; 		//HMI
extern uint8_t First_Cycle_Flag;
extern CAN_data CAN_1;
extern uint8_t ID_CAN;

#define FRAME_START_BYTE 0xAA
#define FRAME_SIZE 12  

/**********************************************************************************************/
void RS485_Init(void)
{
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
  HAL_UARTEx_ReceiveToIdle_DMA(&huart4,usart4_Rx.rxBuff, 100);
	HAL_Delay(1000);
}

/**********************************************************************************************/
uint8_t calculate_checksum(uint8_t *data, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

void process_RS485_frame(usartData *pRxData,CAN_data *CAN) {
    if (pRxData->rxBuff[0] != FRAME_START_BYTE) {
        return;  
    }

    uint8_t checksum = calculate_checksum(pRxData->rxBuff, 11);
    if (checksum != pRxData->rxBuff[11]) {
        return;  
    }

    uint8_t sender = pRxData->rxBuff[1];
    uint8_t receiver = pRxData->rxBuff[2];
    float f1, f2;

    memcpy(&f1, &pRxData->rxBuff[3], sizeof(float));
    memcpy(&f2, &pRxData->rxBuff[7], sizeof(float));
    if(ID_CAN == 0x11){
		CAN->IaSP=f1;
		}
    else{
		CAN->IaSP=f2;
		}
}
/**********************************************************************************************/
void vUsart_DataInit(usartData *pData)
{
}
/***************************************UART4 - RS485 PORT*********************************************/
void vUsart4_RxHandle(usartData *pRxData)
{
	
}
/*************************************UART5 - Remote PC Command Handle*****************************************/
void vUsart5_RxHandle(usartData *pRxData)
{
	
}
/**********************************************************************************************/
void vUsart_TxHandle(usartData *pTxData, uint8_t Port, uint8_t messType) //For Test Purpose
{	
	
}
/**********************************************************************************************/
void vUsart_Read(usartData *pRxData, uint8_t Port)	//Read Data from serial port
{
	
}
/**********************************************************************************************/
void vUSART_RxTimeOutHandle(usartData *pRxData)
{
	
}
/**********************************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */

	if(huart->Instance == UART4)											//RS485
	{
//		vUsart_Read(&usart4_Rx, 4);
//		HAL_UARTEx_ReceiveToIdle_IT(&huart4,usart4_Rx.rxBuff, 100);	
	}
	if(huart->Instance == UART5)											//PC
	{
		vUsart_Read(&usart5_Rx, 5);
		HAL_UART_Receive_IT (&huart5, &usart5_Rx.rxByte, 1);	
	}
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  UNUSED(Size);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UARTEx_RxEventCallback can be implemented in the user file.
   */
	huart->ErrorCode = HAL_UART_ERROR_NONE;
		if(huart->Instance == UART4)											//RS485
	{
		process_RS485_frame(&usart4_Rx,&CAN_1);
		HAL_UARTEx_ReceiveToIdle_DMA(&huart4,usart4_Rx.rxBuff, 100);	
	}
}
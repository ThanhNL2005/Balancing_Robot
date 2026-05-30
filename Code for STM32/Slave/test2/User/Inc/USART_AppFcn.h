#include "stm32f1xx_hal.h"
#include "Analog_IO.h"
#include "MachineControl.h"
#include "Nextion.h"
/**********************************************************************************************/
//usart data
typedef struct
{
	//Buffer
	uint8_t rxBuff[100];
	uint8_t txBuff[100];
	uint8_t NEXTION_Buff[10];
	//Header
	uint8_t frameLength, groupID, senderAddr, receiverAddr, messType, dataLength;
	uint16_t messCount;
	//Data payload
	
	//State flow 
	uint8_t STATE;
	uint8_t RxFlag;
	uint8_t rxByte, rxPointer;
	uint8_t NEXTION_Pointer, NEXTION_Flag, NEXTION_RetCode;	
	uint16_t timeOut; 
}usartData;

void vUsart2_rxHandle(usartData *pRxData);													//Response to received message
void vUsart4_RxHandle(usartData *pRxData, usartData *pTxData, machine_Control *pMControl, adc_Objt *pAdc);													//Response to received message
void vUsart5_rxHandle(usartData *pRxData, NEXTION *pNEXVar, machine_Control *pStateMachine);

void vUsart_TxHandle(adc_Objt *pADC, machine_Control *pStateMachine, 
	RAMP *pRAMP_Ia, RAMP *pRAMP_Speed, usartData *pTxData, uint8_t Port, uint8_t messType); //For Test Purpose

void vUsart2_TxHandle(adc_Objt *pADC, machine_Control *pStateMachine, 
	RAMP *pRAMP1, RAMP *pRAMP2, usartData *pTxData, uint8_t messType);//Process and send data to ESP32
void vUsart4_TxHandle(adc_Objt *pADC, machine_Control *pStateMachine, 
	RAMP *pRAMP1, RAMP *pRAMP2, usartData *pTxData, uint8_t messType);//Process and send data to RS485
void vUsart5_TxHandle(adc_Objt *pADC, machine_Control *pStateMachine, 
	RAMP *pRAMP1, RAMP *pRAMP2, usartData *pTxData, uint8_t messType);//Process and send data to HMI		

void vUsart_DataInit(usartData *pData);
void vUsart_Read(usartData *pRxData, uint8_t Port);									//Read Data from ESP32
void vUSART_RxTimeOutHandle(usartData *pRxData);

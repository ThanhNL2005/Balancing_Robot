#include "Communication.h"
#include "stm32f4xx_hal.h"           // Include STM32 HAL library for H7 series
#include "stm32f4xx_hal_can.h"     // Include STM32 HAL FDCAN library


extern CAN_RxHeaderTypeDef RxHeader;
extern CAN_TxHeaderTypeDef TxHeader;
extern CAN_HandleTypeDef hcan1;
extern uint32_t TxMailbox;
extern uint8_t ID_CAN;
double test_w[200];
uint8_t size;
 union {
    double d;
    uint8_t bytes[8];
} converter;
uint8_t RxData[8];
uint8_t TxData[8];
CAN_data CAN_1;


void CAN_Init(CAN_HandleTypeDef *p)
{
	p->Instance = CAN1;
  p->Init.Prescaler = 18;
  p->Init.Mode = CAN_MODE_NORMAL;
  p->Init.SyncJumpWidth = CAN_SJW_1TQ;
  p->Init.TimeSeg1 = CAN_BS1_2TQ;
  p->Init.TimeSeg2 = CAN_BS2_1TQ;
  p->Init.TimeTriggeredMode = DISABLE;
  p->Init.AutoBusOff = DISABLE;
  p->Init.AutoWakeUp = DISABLE;
  p->Init.AutoRetransmission = DISABLE;
  p->Init.ReceiveFifoLocked = DISABLE;
  p->Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(p) != HAL_OK)
  {
    Error_Handler();
  }
}

 void CAN_Transmit_Init_Slave1(CAN_TxHeaderTypeDef *p,uint8_t *datalength)
{
p->DLC=*datalength;
p->ExtId=0;
p->IDE=CAN_ID_STD;
p->RTR=CAN_RTR_DATA;
p->StdId=0x11;//0x11;
p->TransmitGlobalTime=DISABLE;
}

void CAN_Transmit_Init_Slave2(CAN_TxHeaderTypeDef *p,uint8_t *datalength)
{
p->DLC=*datalength;
p->ExtId=0;
p->IDE=CAN_ID_STD;
p->RTR=CAN_RTR_DATA;
p->StdId=0x12;
p->TransmitGlobalTime=DISABLE;
}

void CAN_Transmit_Init_Slave3(CAN_TxHeaderTypeDef *p,uint8_t *datalength)
{
p->DLC=*datalength;
p->ExtId=0;
p->IDE=CAN_ID_STD;
p->RTR=CAN_RTR_DATA;
p->StdId=0x13;
p->TransmitGlobalTime=DISABLE;
}

 void CAN_Filter_Slave1_Init(CAN_FilterTypeDef *p)
{
p->FilterActivation = CAN_FILTER_ENABLE;
p->FilterBank = 10;
p->FilterFIFOAssignment = CAN_RX_FIFO0;
p->FilterIdHigh = 0x10<<5;//0x10<<5
p->FilterIdLow = 0x0000; 
p->FilterMaskIdHigh =0x13<<5;//0x13<<5;
p->FilterMaskIdLow = 0x0000;  
p->FilterMode = CAN_FILTERMODE_IDMASK;  
p->FilterScale = CAN_FILTERSCALE_32BIT; 
p->SlaveStartFilterBank =14;
}

void CAN_Filter_Slave2_Init(CAN_FilterTypeDef *p)
{
p->FilterActivation = CAN_FILTER_ENABLE;
p->FilterBank = 10;
p->FilterFIFOAssignment = CAN_RX_FIFO0;
p->FilterIdHigh = 0x10<<5;//0x10<<5
p->FilterIdLow = 0x0000; 
p->FilterMaskIdHigh =0x13<<5;//0x13<<5
p->FilterMaskIdLow = 0x0000;  
p->FilterMode = CAN_FILTERMODE_IDMASK;  
p->FilterScale = CAN_FILTERSCALE_32BIT; 
p->SlaveStartFilterBank =14;
}

void CAN_Filter_Slave3_Init(CAN_FilterTypeDef *p)
{
p->FilterActivation = CAN_FILTER_ENABLE;
p->FilterBank = 10;
p->FilterFIFOAssignment = CAN_RX_FIFO0;
p->FilterIdHigh = 0x10<<5;//0x10<<5
p->FilterIdLow = 0x0000; 
p->FilterMaskIdHigh =0x13<<5;//0x13<<5
p->FilterMaskIdLow = 0x0000;  
p->FilterMode = CAN_FILTERMODE_IDMASK;  
p->FilterScale = CAN_FILTERSCALE_32BIT; 
p->SlaveStartFilterBank =14;
}
void conv_float_to_uint(volatile float*p1,char*p2, uint8_t*p3)
{   
    sprintf(p2, "%.2f",  *p1); 
    p3[0]=(uint8_t)p2[0];
    p3[1]=(uint8_t)p2[1];
    p3[2]=(uint8_t)p2[2];
    p3[3]=(uint8_t)p2[3];
    p3[4]=(uint8_t)p2[4];
		p3[5]=(uint8_t)p2[5];
		p3[6]=(uint8_t)p2[6];
}
void conv_uint_to_float(volatile float*p1,char*p2, uint8_t*p3)
{
    p2[0]=(char)p3[0];
    p2[1]=(char)p3[1];
    p2[2]=(char)p3[2];
    p2[3]=(char)p3[3];
    p2[4]=(char)p3[4];
		p2[5]=(char)p3[5];
		p2[6]=(char)p3[6];
    *p1=atof(p2);
}

void UART_Init(UART* p){
	for(int i=0;i<MAX_DATA;i++){
		 p->Ia[i]=0;
	}
	p->Ia_cnt=0;
	p->done=0;
}
void Save_Data(UART* p,float Ia_filter){
	if(p->Ia_cnt<MAX_DATA){
		p->Ia[p->Ia_cnt]=Ia_filter;
		p->Ia_cnt++;
	}
}
void Send_Data(UART* p,UART_HandleTypeDef*huart){
	if(p->Ia_cnt==MAX_DATA-1){
		for (int i=0;i<p->Ia_cnt;i++){
			char data[10];
			sprintf(data,"%f\n",p->Ia[i]);
			HAL_UART_Transmit(huart, (uint8_t *)data, sizeof(data),HAL_MAX_DELAY);
		}
		p->done=1;
	}
}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{	
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)  {
		
	for (int i = 0; i < 8; i++) {
			converter.bytes[i] = RxData[i];
	}
	if ( RxData[7]!=0){
//	 HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); // for debug
	 CAN_1.IaSP = converter.d;
	}
	
	}
}
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)                                                
//{   
//    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)  {
//        static union {
//            float f[2]; 
//            uint8_t bytes[8]; 
//        } converter;
//        for (int i = 0; i < 8; i++) {
//            converter.bytes[i] = RxData[i];
//        }
//        if(ID_CAN==0x11){
//        CAN_1.IaSP = converter.f[0];
//				}
//				else{
//        CAN_1.IaSP = converter.f[1];
//        }
//				test_w[size]=CAN_1.IaSP;
//				size++;
//        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); // Debug
//    }
//}
void send_data_can(const double data, uint8_t ID){
		union {
    double d;
    uint8_t bytes[8];
} double_to_byte;
	TxHeader.StdId = ID;
	double_to_byte.d=data;
		for (int i = 0; i < 8; i++) {
			TxData[i]=double_to_byte.bytes[i] ;}
        /* Start the Transmission process */
        if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
        {
          Error_Handler();
        }
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
	
}


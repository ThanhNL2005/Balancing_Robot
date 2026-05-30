#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#define MAX_DATA 5000

void CAN_Init(CAN_HandleTypeDef *p);
void CAN_Transmit_Init_Slave1(CAN_TxHeaderTypeDef *p,uint8_t *datalength);
void CAN_Transmit_Init_Slave2(CAN_TxHeaderTypeDef *p,uint8_t *datalength);
void CAN_Transmit_Init_Slave3(CAN_TxHeaderTypeDef *p,uint8_t *datalength);
void CAN_Filter_Slave1_Init(CAN_FilterTypeDef *p);
void CAN_Filter_Slave2_Init(CAN_FilterTypeDef *p);
void CAN_Filter_Slave3_Init(CAN_FilterTypeDef *p);
void conv_float_to_uint(volatile float*p1,char*p2, uint8_t*p3);
void conv_uint_to_float(volatile float*p1,char*p2, uint8_t*p3);

// Transmit current value to PC
typedef struct{
	float Ia[MAX_DATA];
	int Ia_cnt; 
  uint8_t done;
}UART;
void UART_Init(UART* p);
void Save_Data(UART* p,float Ia_filter);
void Send_Data(UART* p,UART_HandleTypeDef*huart);
#endif
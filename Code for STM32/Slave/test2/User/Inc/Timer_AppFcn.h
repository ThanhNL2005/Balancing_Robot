#include "stm32f1xx_hal.h"
#include "main.h"
/**********************************************************************************************/
//Timer 
typedef struct 
{
	uint16_t En;	//Enable the timer
	uint16_t SV;	//Set value
	uint16_t PV; 	//Present value
	uint16_t Output;
	uint16_t OvfCnt;
}timer_Objt;	
/**********************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);			//Timer interrupt handle
void vTim_Start(timer_Objt *ptim, uint16_t SV);				//Start timer 10ms
void vTim_Stop(timer_Objt *ptim);											//Reset timer 10ms
void vTim_Scan(timer_Objt *pTIM);
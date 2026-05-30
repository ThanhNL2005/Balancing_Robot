#include "main.h"

//GPIO object
extern gpio_Objt DI_Objt[NumOfDI];
/**********************************************************************************************/
void vDI_Scan(void)	//Scan the proximity sensors in operation, every 10ms
{
	//Read all DI
	DI_Objt[0].dataK = HAL_GPIO_ReadPin(SERVO_LOCK_GPIO_Port, SERVO_LOCK_Pin);	
	DI_Objt[1].dataK = HAL_GPIO_ReadPin(DI_1_GPIO_Port, DI_1_Pin);	//RUN/STOP Cmd	
	DI_Objt[2].dataK = HAL_GPIO_ReadPin(DI_2_GPIO_Port, DI_2_Pin);	//Direction
		
	//Event detect
	DI_Objt[0].event = eDI_EventDetect(DI_Objt[0].dataK, DI_Objt[0].dataK_1);
	DI_Objt[1].event = eDI_EventDetect(DI_Objt[1].dataK, DI_Objt[1].dataK_1);
	DI_Objt[2].event = eDI_EventDetect(DI_Objt[2].dataK, DI_Objt[2].dataK_1);
	
	//Update DI
	DI_Objt[0].dataK_1 = DI_Objt[0].dataK;
	DI_Objt[1].dataK_1 = DI_Objt[1].dataK;
	DI_Objt[2].dataK_1 = DI_Objt[2].dataK;
}
/**********************************************************************************************/
void vDIO_Init(void)	//Intialize DIO at start-up
{
	//Read all DI
	DI_Objt[0].dataK = HAL_GPIO_ReadPin(SERVO_LOCK_GPIO_Port, SERVO_LOCK_Pin);
	DI_Objt[1].dataK = HAL_GPIO_ReadPin(DI_1_GPIO_Port, DI_1_Pin);
	DI_Objt[2].dataK = HAL_GPIO_ReadPin(DI_2_GPIO_Port, DI_2_Pin);

	//Update DI
	DI_Objt[0].dataK_1 = DI_Objt[0].dataK;
	DI_Objt[1].dataK_1 = DI_Objt[1].dataK;
	DI_Objt[2].dataK_1 = DI_Objt[2].dataK;
}
/**********************************************************************************************/
gpio_Event eDI_EventDetect(GPIO_PinState k_Instance, GPIO_PinState k_1_Instance) //Check the event
{
	gpio_Event temp;
	if(k_Instance == GPIO_PIN_SET && k_1_Instance == GPIO_PIN_SET)
		temp = High;
	else if(k_Instance == GPIO_PIN_SET && k_1_Instance  == GPIO_PIN_RESET)
		temp = Rising;
	else if(k_Instance == GPIO_PIN_RESET && k_1_Instance == GPIO_PIN_SET)
		temp = Falling;
	else
		temp = Low;
	return temp;
}
/**********************************************************************************************/
void GPIO_Test(void) 	//For test only
{
	uint16_t i = 0;
	for(i = 0;i<NumOfDI;i++) 
	{ 
		//Detect rising and falling edge of DI, then Toggle all DO
		if((DI_Objt[i].event == Rising) || (DI_Objt[i].event == Falling))
		{
			HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
			HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		}
	}
}
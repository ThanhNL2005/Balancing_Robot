#include "stm32f1xx_hal.h"
#include "main.h"
/**********************************************************************************************/
typedef enum  
{
	Low,
	High, 
	Rising, 
	Falling
}gpio_Event;
/**********************************************************************************************/
//GPIO
typedef struct 
{
	GPIO_PinState dataK, dataK_1;	
	gpio_Event event;
}gpio_Objt;

void vDIO_Init(void);			//Scan all input and setup output at startup
void vDI_Scan(void);			//Scan the proximity sensors in operation, every 10ms
gpio_Event eDI_EventDetect(GPIO_PinState k_Instance, GPIO_PinState k_1_Instance); //Check the even
void vGPIO_Test(void); 	//For test only
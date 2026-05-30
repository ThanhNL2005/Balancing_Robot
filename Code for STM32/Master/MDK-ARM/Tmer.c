#include "main.h"

/*Timer object*/
extern uint16_t tim10msTick;
uint8_t GPIO[3];
extern int start,control;
// HSMC control
extern TWBMR parameter_TWBMR;
extern HSMC control_hsmc,control_smc;
extern F f_funtion;
extern G g_funtion;
//esp32
extern ESP32 esp32;
extern uint8_t t_esp32;
extern uint8_t frame[19] ;
// PID
extern cPI pid_x;
extern cPI pid_theta;
extern cPI pid_psi;
extern uint8_t frame[19] ;
	

/**********************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);		
	/*USER CODE*/
	if(htim->Instance == TIM5) //PWM update interrupt - Ts = 50us, Fsw = 10kHz
	{
		/*For test only*/	
		//HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);	
		/*Machine control*/	
//		if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_8)){
//		NVIC_SystemReset();			
//		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,GPIO_PIN_RESET);
//		}
		GPIO[0]=HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9);
		GPIO[1]=HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_8);
		GPIO[2]=HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10);
//		if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9)){
//			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,GPIO_PIN_SET);
		if(start == 1 && control == 1){
				if(esp32.rec_mode)ESP_to_STM(frame,&pid_x,&pid_theta,&pid_psi,&esp32);
//		  control_theta_pid();
//			control__pid();
//		  HSMC_control(&parameter_TWBMR,&control_hsmc,&f_funtion,&g_funtion);
		  SMC_control(&parameter_TWBMR,&control_smc,&f_funtion,&g_funtion);
		}
	
		//Scan the 10ms timer
		if(++tim10msTick == 999)
		{
				t_esp32=1;
				tim10msTick = 1;
		}
	}
		

	
}

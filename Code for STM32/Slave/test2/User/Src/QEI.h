#include "stm32f1xx_hal.h"
#include "Analog_IO.h"
/**********************************************************************************************/
//Quadrature Encoder
typedef struct
{
	int16_t ppr;								//Pulse per round of the Encoder
	float Ts;										//Sampling time (sec)
	int16_t Cnt_k, Cnt_k_1;			//Value of counter at time instance k and k_1
	uint16_t Cnt_Ovf; 					//Overflow counter
	int16_t rpm;								//Round per minute of the motor - need to be calculated
	int8_t Direction;						//Direction of the motor
	int8_t PU_rpm;
	int8_t FirstCnt;			
}QEI;

//Quadrature Encoder Functions
void vQEI_Config(QEI *pQEI, int ppr, int mode, float Ts);
void vRPM_Cal(QEI *pQEI, adc_Objt *pADC);
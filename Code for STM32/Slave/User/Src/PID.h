#include "stm32f1xx_hal.h"
#include "main.h"
/**********************************************************************************************/
//PI Controller
typedef struct
{
	float Kp, KI, TI;								//Controller parameters	
	float Ks, D;										//Ks: Anti-windup Gain; D = 1-Ts/TI
	float Umax, Umin, Usk, Usk_1, Uk, Uk_1;	//Control signal
	float Ek, Ek_1, Esk, Esk_1;			//Error
	float Ts;												//Sampling time
	uint8_t Mode;										//Mode = 0: P, Mode = 1: PI
}cPI;
/**********************************************************************************************/
void vPI_Init(cPI *pPI, float KI, float KP, float Ts, float MaxSP, float Usat);	//PI Controller with Anti-Windup
float fPI_Run(cPI *pPI, float Ref, float Fb);
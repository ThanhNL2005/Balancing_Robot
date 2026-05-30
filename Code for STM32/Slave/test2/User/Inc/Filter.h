#include "stm32f1xx_hal.h"
#include "main.h"
/**********************************************************************************************/
//Second-Order Unity-Gain Low Pass Filter
typedef struct
{
	float Ts;							//Sampling time
	float Fc; 						//Conrner frequency (Hz)
	float a1, a2, b1, b2;	//Parameters of the filter
	float alpha;
	float yk, yk_1, yk_2, uk, uk_1;
}LPF;

void vLPF_Init(LPF *pLPF, uint16_t ConnerFrqHz, float samplingTime);	//Initialize LPF 
float fLPF_Run(LPF *pLPF, float input);																//Run LPF 
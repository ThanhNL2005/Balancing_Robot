#include "main.h"

//2nd Order Unity-Gain LPF
extern LPF stLPF_Var[NumOfLPF];
extern NotchFilter stNotchFilter_Var;

void v_NotchFilter_Init(NotchFilter *NFvar, float f_Center_Frq, float f_Sampling_Frq)
{
	/*Notch filter parameters*/
	NFvar->f0 = f_Center_Frq; //Hz
	NFvar->fs = f_Sampling_Frq;
	NFvar->omega = 2*3.141*NFvar->f0/NFvar->fs;
	NFvar->r = 0.99;

	NFvar->b0 = 1;
	NFvar->b1 = -2*cos(NFvar->omega);
	NFvar->b2 = 1;
	NFvar->a0 = 1;
	NFvar->a1 = -2*NFvar->r*cos(NFvar->omega);
	NFvar->a2 = NFvar->r*NFvar->r;

	NFvar->G = (NFvar->a0 + NFvar->a1 + NFvar->a2)/(NFvar->b0 + NFvar->b1 + NFvar->b2);

	NFvar->yk = 0;
	NFvar->yk_1 = 1; 
	NFvar->yk_2 = 2; 
	NFvar->uk = 0;
	NFvar->uk_1 = 0;
	NFvar->uk_2 = 0;
}
/**********************************************************************************************/
/*Notch Filter Run*/
float f_NotchFilter_Run(NotchFilter *NFvar, float Input)
{
	NFvar->uk = Input;
	NFvar->yk = (1/NFvar->a0)*(-NFvar->a1*NFvar->yk_1 -NFvar->a2*NFvar->yk_2 
	+NFvar->G*NFvar->b0*NFvar->uk + NFvar->G*NFvar->b1*NFvar->uk_1+NFvar->G*NFvar->b2*NFvar->uk_2);

	NFvar->yk_2 = NFvar->yk_1; 
	NFvar->yk_1 = NFvar->yk; 
	NFvar->uk_2 = NFvar->uk_1; 
	NFvar->uk_1 = NFvar->uk;
	return NFvar->yk;
}
/**********************************************************************************************/
void vLPF_Init(LPF *pLPF, uint16_t ConnerFrqHz, float samplingTime)
{
	/*Low pass filter*/
	float temp1, temp2;
	
	pLPF->Fc = ConnerFrqHz;
	pLPF->Ts = samplingTime;
	pLPF->alpha = 1.0f/(2*PI*pLPF->Ts*pLPF->Fc);
	
	temp1 = (1+pLPF->alpha); 
	temp2 = temp1*temp1;
	pLPF->a1 = (1+2*pLPF->alpha)/temp2; 
	pLPF->a2 = -2*pLPF->alpha/temp2;
	pLPF->b1 = -2*pLPF->alpha/temp1; 
	pLPF->b2 = pLPF->alpha*pLPF->alpha/temp2;
	
	pLPF->yk = 0;
	pLPF->yk_1 = 0; 
	pLPF->yk_2 = 0;
	pLPF->uk = 0;
	pLPF->uk_1 = 0;
}
/**********************************************************************************************/
float fLPF_Run(LPF *pLPF, float input)
{
	//Read LPF input
	pLPF->uk = input;	
	//Compute LPF output
	pLPF->yk = -pLPF->b1*pLPF->yk_1 - pLPF->b2*pLPF->yk_2 + pLPF->a1*pLPF->uk + pLPF->a2*pLPF->uk_1; 
	//Save LPF past data
	pLPF->yk_2 = pLPF->yk_1; 
	pLPF->yk_1 = pLPF->yk; 
	pLPF->uk_1 = pLPF->uk;
	//Return LPF output
	return pLPF->yk;
}
#include "main.h"

float fRampGen(RAMP *pRamp,float SP, float delatl){
pRamp->f_delta=delatl;
	pRamp->f_SPk=SP;
	if(pRamp->f_SPk > pRamp->f_Output + pRamp->f_delta)
	{
	pRamp->f_Output = pRamp->f_Output + pRamp->f_delta;
	}
	else if (pRamp->f_SPk < pRamp->f_Output - pRamp->f_delta)
	{
	pRamp->f_Output = pRamp->f_Output - pRamp->f_delta;
	}
	else 
	{
		pRamp->f_Output=pRamp->f_SPk;
	}
	return pRamp->f_Output;
	
}
/**********************************************************************************************/
void vRamp_Config(RAMP *pRamp, uint16_t Tacc_Sec, uint16_t Tdec_Sec, float inputMin, float inputMax, float Ts)
{
	pRamp->f_inputMax = inputMax;
	pRamp->f_inputMin = inputMin;
	pRamp->ui16_T_Acc = Tacc_Sec;
	pRamp->ui16_T_Dec = Tdec_Sec;
	
	//Compute the set count - based on the sampling time Ts
	pRamp->ui16_set_AccCnt = pRamp->ui16_T_Acc/Ts;	
	pRamp->ui16_set_DecCnt = pRamp->ui16_T_Dec/Ts;
	//Compute the step size
	pRamp->f_delta_Acc = (float)(pRamp->f_inputMax - pRamp->f_inputMin)/pRamp->ui16_set_AccCnt;
	pRamp->f_delta_Dec = (float)(pRamp->f_inputMax - pRamp->f_inputMin)/pRamp->ui16_set_DecCnt;
	
	pRamp->f_SPk = 0.0f;
	pRamp->f_Output = 0.0f;
}
/**********************************************************************************************/
float fRampFcnGen(float SP, RAMP *pRamp)
{
	pRamp->f_SPk = SP;
	//Detect the state
	if(pRamp->f_SPk > pRamp->f_Output + pRamp->f_delta_Acc)
	{
		pRamp->enum_State = ACC;
	}
	else if (pRamp->f_SPk < pRamp->f_Output - pRamp->f_delta_Dec)
	{
		pRamp->enum_State = DEC;
	}
	else
	{
		pRamp->enum_State = RUN;
	}
	//Generate output
	if(pRamp->enum_State == ACC)
	{
		pRamp->f_Output = pRamp->f_Output + pRamp->f_delta_Acc;
		if(pRamp->f_Output >= pRamp->f_SPk)	//End the acceleration
		{
			pRamp->f_Output = pRamp->f_SPk;
			pRamp->enum_State = RUN; 					
		}
	}
	else if(pRamp->enum_State == DEC)
	{
		pRamp->f_Output = pRamp->f_Output - pRamp->f_delta_Dec;
		if(pRamp->f_Output <= pRamp->f_SPk)	//End the deceleration
		{
			pRamp->f_Output = pRamp->f_SPk;
			pRamp->enum_State = RUN; 					
		}
	}
	else if(pRamp->enum_State == RUN)
	{
		pRamp->f_Output = pRamp->f_SPk;
	}
	return pRamp->f_Output;
}


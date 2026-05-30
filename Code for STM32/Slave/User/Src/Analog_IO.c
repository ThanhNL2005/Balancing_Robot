#include "main.h"

/*ADC data*/
extern volatile adc_Objt adc_Objt_var;
extern volatile uint16_t ADC_CH1[NumOfADC_Channel];
extern ADC_HandleTypeDef hadc1;
/*DAC data*/
extern DAC_OUT stDAC_Var;
extern DAC_HandleTypeDef hdac;

//2nd Order Unity-Gain LPF
extern LPF stLPF_Var[NumOfLPF];

//State machine
extern machine_Control stMControl_Var;		
extern machine_Parameter stMachine_Var;

//2nd Order Unity-Gain LPF
extern LPF st_LPF_IaSP;
extern LPF st_LPF_SpdSP;
extern LPF st_LPF_Ia;
extern LPF st_LPF_Vbus;
extern LPF st_LPF_Vref_165;
extern LPF st_LPF_AD_Ia_Offset;
/**********************************************************************************************/
void vAdc_DataInit(adc_Objt *pObjt, machine_Parameter *pMParas)			//Initialize adc data
{
	int i; 
	for(i=0;i<NumOfADC_Channel;i++)
	{
		pObjt->AD1_Data[i] = 0;
	}	
	pObjt->f_MaxVbus = 44.55;	//Vol
	pObjt->f_Ia_Sensitivity = 0.1381f; //0.2556f;	/*V/A*/
	pObjt->f_AD_Vref = 3.3f; 									//V
	pObjt->f_Vbus_Divider = 104.7f/4.7f;			//Dependence on hardware
	pObjt->f_Vbus_G = pObjt->f_Vbus_Divider*pObjt->f_AD_Vref/AD_MAX_12Bit; 		//Fixed Gain 
	pObjt->f_Ia_G = pObjt->f_AD_Vref/(pObjt->f_Ia_Sensitivity*AD_MAX_12Bit); 	//Fixed Gain
		
	pObjt->ui16_AD_Speed_SP_Offset = 0; 
	pObjt->ui16_AD_Torque_SP_Offset = 0;
	
	pObjt->f_KFactor_Ia = 1.0f;
	pObjt->f_KFactor_Speed_SP = 1.0f;
	pObjt->f_KFactor_Torque_SP = 1.0f;
	pObjt->f_KFactor_Vbus = 1.0f;
}
/**********************************************************************************************/
void vAdc_Data_Handle(adc_Objt *pObjt, machine_Parameter *pMParas, setpoint *pSPVar)
{
	int16_t AD_IaSP, AD_SPD_SP, AD_1_65Vref, AD_IaSense, AD_VP, AD_VN;
	
	AD_IaSP = ADC_CH1[0];
	AD_SPD_SP = ADC_CH1[1];
	AD_1_65Vref = ADC_CH1[2];
	AD_IaSense = ADC_CH1[3];
	AD_VP = ADC_CH1[4];
	AD_VN = ADC_CH1[5];
	
	//VBus Computation 
	pObjt->f_Vbus_Raw = pObjt->f_Vbus_G*pObjt->f_KFactor_Vbus*(AD_VP - AD_VN); 					//Vol
	pObjt->f_Vbus_Filter = fLPF_Run(&st_LPF_Vbus, pObjt->f_Vbus_Raw);
	//Ia Compute
	pObjt->i_Ia_Raw = (AD_IaSense - pObjt->ui16_AD_Ia_Offset);
	pObjt->f_Ia_Raw = pObjt->f_Ia_G*pObjt->f_KFactor_Ia*pObjt->i_Ia_Raw; 	//Ampere	
	pObjt->f_Ia_Filter = fLPF_Run(&st_LPF_Ia, pObjt->f_Ia_Raw);
	//Speed Setpoint from Analog Input Computation
	pObjt->f_PU_AN_SpdSP_raw = (float)(AD_SPD_SP)/AD_MAX_12Bit; 						//Per unit
	pObjt->f_PU_AN_SpdSP_Filter = fLPF_Run(&st_LPF_SpdSP, pObjt->f_PU_AN_SpdSP_raw);
	pSPVar->f_PU_AN_SpdSP = pObjt->f_PU_AN_SpdSP_Filter; 										//Per unit
	pSPVar->f_SI_AN_MotorShaft_SpdSP = pSPVar->f_PU_AN_SpdSP*pMParas->f_Motor_Spd_max;	//Motor Shaft RPM
	pSPVar->f_SI_AN_GearShaft_SpdSP = pSPVar->f_SI_AN_MotorShaft_SpdSP/pMParas->f_Gear_Ratio;	//Motor Shaft RPM
	//Amature voltage setpoint from Analog Input
	pSPVar->f_PU_AN_VaSP = pSPVar->f_PU_AN_SpdSP ; //Per unit
	pSPVar->f_SI_AN_VaSP = pSPVar->f_PU_AN_VaSP*pMParas->f_Va_rated; //Vol	
	//Amature current Setpoint from Analog Input
	pObjt->f_PU_AN_IaSP_raw = (float)(AD_IaSP)/AD_MAX_12Bit;	 			//Per unit
	pObjt->f_PU_AN_IaSP_Filter = fLPF_Run(&st_LPF_IaSP, pObjt->f_PU_AN_IaSP_raw);
	pSPVar->f_PU_AN_IaSP = pObjt->f_PU_AN_IaSP_Filter;	 						//Per unit
	pSPVar->f_SI_AN_IaSP = pSPVar->f_PU_AN_IaSP*pMParas->f_Ia_max;	//Ampere
}
/**********************************************************************************************/
void vDAC_Init(DAC_OUT *pAO)																				//DAC data initialize
{
	pAO->ui_AO1 = 0;
	pAO->ui_AO2 = 0;
	pAO->f_Offset1 = 1.65f;
	pAO->f_Offset2 = 1.65f;
	pAO->f_VO1 = 0.0f;
	pAO->f_VO2 = 0.0f;
	pAO->f_Range1_Min = -1.65f;
	pAO->f_Range1_Max = 1.65f;
	pAO->f_Range2_Min = -1.65f;
	pAO->f_Range2_Max = 1.65f;
	pAO->f_Sensitivity = 0.225;	//Vol per Ampere
}
/**********************************************************************************************/
void vDAC_CH1_IaWrite(DAC_OUT *pAO, uint8_t DAC_Channel, float Ia)
{
	float Vout;
	Vout = Ia*pAO->f_Sensitivity;
	switch(DAC_Channel)
	{
		case 1:
		{
			if(Vout > pAO->f_Range1_Max)
				pAO->f_VO1 = pAO->f_Range1_Max;
			else if(Vout < pAO->f_Range1_Min)
				pAO->f_VO1 = pAO->f_Range1_Min;
			else
				pAO->f_VO1 = Vout;
			pAO->ui_AO1 = ((pAO->f_VO1 + pAO->f_Offset1)/(pAO->f_Range1_Max-pAO->f_Range1_Min))*4095;
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, pAO->ui_AO1);		
			break;
		}
		case 2:
		{
			if(Vout > pAO->f_Range2_Max)
				pAO->f_VO2 = pAO->f_Range2_Max;
			else if(Vout < pAO->f_Range2_Min)
				pAO->f_VO2 = pAO->f_Range2_Min;
			else
				pAO->f_VO2 = Vout;
			pAO->ui_AO2 = ((pAO->f_VO2 + pAO->f_Offset2)/(pAO->f_Range2_Max-pAO->f_Range2_Min))*4095;
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, pAO->ui_AO2);
			break;
		}
	}
}
/**********************************************************************************************/
void vDAC_Write(DAC_OUT *pAO, uint8_t DAC_Channel, float Vout)			//DAC write, Vout = 0V->3V
{
	switch(DAC_Channel)
	{
		case 1:
		{
			if(Vout > pAO->f_Range1_Max)
				pAO->f_VO1 = pAO->f_Range1_Max;
			else if(Vout < pAO->f_Range1_Min)
				pAO->f_VO1 = pAO->f_Range1_Min;
			else
				pAO->f_VO1 = Vout;
			pAO->ui_AO1 = ((pAO->f_VO1 + pAO->f_Offset1)/(pAO->f_Range1_Max-pAO->f_Range1_Min))*4095;
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, pAO->ui_AO1);		
			break;
		}
		case 2:
		{
			if(Vout > pAO->f_Range2_Max)
				pAO->f_VO2 = pAO->f_Range2_Max;
			else if(Vout < pAO->f_Range2_Min)
				pAO->f_VO2 = pAO->f_Range2_Min;
			else
				pAO->f_VO2 = Vout;
			pAO->ui_AO2 = ((pAO->f_VO2 + pAO->f_Offset2)/(pAO->f_Range2_Max-pAO->f_Range2_Min))*4095;
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, pAO->ui_AO2);
			break;
		}
	}
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
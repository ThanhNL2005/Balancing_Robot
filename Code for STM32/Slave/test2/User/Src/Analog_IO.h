#include "stm32f1xx_hal.h"
#include "MachineControl.h"

//Analog input
typedef struct 
{
	uint16_t data[5];	
	float calibVol0, calibVol1, calibVol6, calibVol7, calibVol14;	
	float rawVol0, rawVol1, rawVol6, rawVol7, rawVol14;
	float filterVol0, filterVol1, filterVol6, filterVol7, filterVol14;
	float Gain1, Gain2, ExVref;
	float Offset_Ia, Offset_If, Offset_Vbus;			//Zero point of corresponding channel
	float filterVol_Vbus, filterVol_Ia, filterVol_If, filterVol_SpeedSP, filterVol_TorqueSP;
	
	float Vbus, If, Torque;
	float Iak, Iak_1, dIak_dt, filtered_dIak_dt;
	float PU_Vbus, PU_Torque, PU_If, PU_Ia;
	float IfSP, IaSP, SpeedSP, TorqueSP;
	float MaxVol_TorqueSP, MaxVol_SpeedSP;
	
	float calibIa, calibIf, calibSpeedSP, calibTorqueSP, calibVbus;
	float MaxVbus, MaxIa, MaxIf, MaxTorque; 					//Max range of Sensors
	float MaxIfSP, MaxIaSP, MaxSpeedSP, MaxTorqueSP;						//Max range of reference
	float PU_IfSP, PU_IaSP, PU_SpeedSP, PU_TorqueSP;	//PU is computed based on Max range of reference
	
	float IaToTorqueGain;
	float adcVref;
}adc_Objt;

//Analog output
typedef struct 
{
	float Range1, Range2;				//Max range of the output voltage
	float Offset1, Offset2;			
	float VO1, VO2;							//Output voltage: -5V -> 5V
	uint16_t AO1, AO2;					//Value to be sent to the DAC
}DAC_OUT;

void vAdc_Data_Handle(adc_Objt *pObjt, machine_Parameter *pMParas);
void vAdc_DataInit(adc_Objt *pObjt, machine_Parameter *pMParas);			//Initialize adc data

void vDAC_Init(DAC_OUT *pAO);																					//DAC data initialize
void vDAC_Write(DAC_OUT *pAO, uint8_t DAC_Channel, float Vout);				//DAC write, Vout = -2.5V->2.5V
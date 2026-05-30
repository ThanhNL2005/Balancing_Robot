#include "main.h"
//Quadrature Encoder
extern QEI stQEI_Var;	

extern TIM_HandleTypeDef htim3;
/**********************************************************************************************/
void vQEI_Config(QEI *pQEI, int QEI_ppr, int QEI_mode, float f_GearRatio, float Ts) 
{
	//mode = 1, 2, or 4 (x1, x2, x4)
	//ppr: pulse per round
	//Ts: sampling time (second)
	pQEI->ui16t_FirstCnt = 0;				//To ignore the first resulst
	pQEI->ui16t_Cnt_k = 0; 
	pQEI->ui16t_Cnt_Ovf = 0;
	pQEI->ui16t_ppr = QEI_ppr*QEI_mode*f_GearRatio;		
	pQEI->f_PU_rpm = 0;
	pQEI->f_GearShaft_rpm = 0.0f;
	pQEI->f_MotorShaft_rpm = 0.0f;
	pQEI->f_Cnt_CAN = 0.0f;
	pQEI->Ts = Ts;								//second
	pQEI->i16t_Direction = 0;			//Real Direction
	pQEI->Const1 = 60.0f/(Ts*(float)(pQEI->ui16t_ppr));
	pQEI->Auto_Reload = __HAL_TIM_GET_AUTORELOAD(&htim3);
	__HAL_TIM_SET_COUNTER(&htim3, 32000);
}
/**********************************************************************************************/
void vRPM_Cal(QEI *pQEI, machine_Parameter *pMParas)
{
	uint16_t Auto_Reload;
	//Detect the direction of the QEI	
	pQEI->i16t_Direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);
	//Read QEI Counter
	pQEI->ui16t_Cnt_k = __HAL_TIM_GET_COUNTER(&htim3);
	//Compute the rotation speed (rpm)
		__HAL_TIM_SET_COUNTER(&htim3, 32000);
		if(pQEI->ui16t_FirstCnt == 1)
		{
				pQEI->f_GearShaft_rpm = (float)(pQEI->ui16t_Cnt_k-32000)*pQEI->Const1; 	
		}
		else	//Ignore results in first cycle
		{
			pQEI->ui16t_FirstCnt = 1;
			pQEI->f_MotorShaft_rpm = 0;
			pQEI->f_PU_rpm = 0;
			pQEI->f_Cnt_CAN = 0.0f;
		}
		pQEI->f_Cnt_CAN=pQEI->f_Cnt_CAN+(float)(pQEI->ui16t_Cnt_k-32000);

}

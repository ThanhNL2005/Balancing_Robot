#include "main.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1; //RS485
extern UART_HandleTypeDef huart3; //NEXTION HMI
extern UART_HandleTypeDef huart6; //ESP32

//NEXTION HMI
extern NEXTION NEXVar;

/**********************************************************************************************/
void vNEXTION_Init(NEXTION *pNEXVar)
{
	pNEXVar->UpdateEN = 1;
}
/**********************************************************************************************/
void vNEXTION_DisplayHandle(adc_Objt *pADC, machine_Control *pStateMachine, cPI *pSpeed, cPI *pIa, 
	RAMP *pRAMP_Ia, RAMP *pRAMP_Speed, QEI *pQEI, machine_Parameter *pMParas, NEXTION *pNEXVar)
{
	float R = 0;
	if(pNEXVar->UpdateEN == 1)
	{
		//Page number and Xfloat
		pNEXVar->page1_x0_IaFbx10 = pADC->f_Ia_Filter*10;
		//pNEXVar->page1_x2_IExcFbx10 = pADC->If*10;
		//pNEXVar->page1_n0_SpeedFB = pQEI->rpm;
		pNEXVar->page1_n0_SpeedFB = pStateMachine->i_EsSpeed;
		pNEXVar->page1_n2_Vdc = pADC->f_Vbus_Filter;
		
		pNEXVar->page1_x1_IaSPx10 = pStateMachine->f_Ia_SP*10;
		pNEXVar->page1_x3_IExcSPx10 = pStateMachine->f_If_SP*10;
		pNEXVar->page1_n1_SpeedSP = pStateMachine->i_Speed_SP;
		
		//Display Ia on Gauge z0
		R = 270.0f/pMParas->f_Ia_max;
		if(pADC->f_Ia_Filter >= 0 && pADC->f_Ia_Filter < 45.0f/R)	
		{
			pNEXVar->page1_z0_IaFbx10 = 315 + pADC->f_Ia_Filter*R;
		}
		else if(pADC->f_Ia_Filter >= 45.0f/R)
		{
			pNEXVar->page1_z0_IaFbx10 = pADC->f_Ia_Filter*R - 45;
		}
		//Display IExc on Gauge z1
		/*R = 270.0/pADC->MaxIf;
		if(pADC->If >= 0 && pADC->If < 45.0/R)	
		{
			pNEXVar->page1_z1_IExcFbx10 = 315 + pADC->If*R;
		}
		else if(pADC->If >= 45.0/R)
		{
			pNEXVar->page1_z1_IExcFbx10 = pADC->If*R - 45;
		}*/
		//Display rpm on Gauge z2
		R = 270.0f/pMParas->i_Speed_max;
		/*if(pQEI->rpm >= 0 && pQEI->rpm < 45.0/R)	
		{
			pNEXVar->page1_z2_SpeedFB = 315 + pQEI->rpm*R;
		}
		else if(pQEI->rpm>= 45.0/R)
		{
			pNEXVar->page1_z2_SpeedFB = pQEI->rpm*R - 45;
		}*/	
		if(pStateMachine->i_EsSpeed >= 0 && pStateMachine->i_EsSpeed < 45.0f/R)	
		{
			pNEXVar->page1_z2_SpeedFB = 315 + pStateMachine->i_EsSpeed*R;
		}
		else if(pStateMachine->i_EsSpeed>= 45.0f/R)
		{
			pNEXVar->page1_z2_SpeedFB = pStateMachine->i_EsSpeed*R - 45;
		}
		//Display Vdc on Gauge z3
		R = 270.0/pADC->ui16_MaxVbus;
		if(pADC->f_Vbus_Filter >= 0 && pADC->f_Vbus_Filter < 45.0f/R)	
		{
			pNEXVar->page1_z3_Vdc = 315 + pADC->f_Vbus_Filter*R;
		}
		else if(pADC->f_Vbus_Filter >= 45.0f/R)
		{
			pNEXVar->page1_z3_Vdc = pADC->f_Vbus_Filter*R - 45;
		}
		
		if(pStateMachine->enum_Ia_State == ALARM_H)
		{
			vNEXTION_SendString("page1.t6", "IaALARM");
		}
		else if(pStateMachine->enum_Ia_State == FAULT_H)
		{
			vNEXTION_SendString("page1.t6", "IaFAULT");
		}
		else{
			vNEXTION_SendString("page1.t6", "IaNORMAL");
		}
		/*Page 2*/
		pNEXVar->page2_n0_DI0 = HAL_GPIO_ReadPin(SERVO_LOCK_GPIO_Port, SERVO_LOCK_Pin);
		pNEXVar->page2_n1_DI1 = HAL_GPIO_ReadPin(DI_1_GPIO_Port, DI_1_Pin);
		pNEXVar->page2_n2_DI2 = HAL_GPIO_ReadPin(DI_2_GPIO_Port, DI_2_Pin);
		
		pNEXVar->page2_n17_SP2 = pStateMachine->i_Speed_SP;		//rpm
		pNEXVar->page2_n18_RAMP2= pRAMP_Speed->f_Output;			//rpm
		pNEXVar->page2_n19_QEI_DIR = pQEI->Direction; 
		pNEXVar->page2_n20_QEI_CNT = pQEI->Cnt_k;
		pNEXVar->page2_x0_SP1 = pStateMachine->f_Ia_SP*10;		//Ampere
		pNEXVar->page2_x1_RAMP1 = pRAMP_Ia->f_Output*10;			//Ampere
		//pNEXVar->page2_x2_LaxDia_dt = pMParas->La*pADC->filtered_dIak_dt*10;
		
		//HMI Display Control - Page 1
		vNEXTION_SendVal("page1.x0", pNEXVar->page1_x0_IaFbx10, DATA_XFLOAT);
		vNEXTION_SendVal("page1.x2", pNEXVar->page1_x2_IExcFbx10, DATA_XFLOAT);
		vNEXTION_SendVal("page1.n0", pNEXVar->page1_n0_SpeedFB, DATA_INT);
		vNEXTION_SendVal("page1.n2", pNEXVar->page1_n2_Vdc, DATA_INT);
		
		
		vNEXTION_SendVal("page1.z0", pNEXVar->page1_z0_IaFbx10, DATA_INT);
		vNEXTION_SendVal("page1.z1", pNEXVar->page1_z1_IExcFbx10, DATA_INT);
		vNEXTION_SendVal("page1.z2", pNEXVar->page1_z2_SpeedFB, DATA_INT);
		vNEXTION_SendVal("page1.z3", pNEXVar->page1_z3_Vdc, DATA_INT);
		
		vNEXTION_SendVal("page1.x1", pNEXVar->page1_x1_IaSPx10, DATA_XFLOAT);	
		vNEXTION_SendVal("page1.x3", pNEXVar->page1_x3_IExcSPx10, DATA_XFLOAT);	
		vNEXTION_SendVal("page1.n1", pNEXVar->page1_n1_SpeedSP, DATA_INT);	
		
		if(pStateMachine->enum_chState == CHARGE)
		{
			vNEXTION_SendString("page1.t14", "CHARGE");
		}
		else if(pStateMachine->enum_chState == DOL)
		{
			vNEXTION_SendString("page1.t14", "DOL");
		}
		
		if(pStateMachine->enum_mState == mSTANDSTILL)
		{
			vNEXTION_SendString("page1.t10", "IDLE");
		}
		else if(pStateMachine->enum_mState == mSTOP)
		{
			vNEXTION_SendString("page1.t10", "STOP");
		}
		else if(pStateMachine->enum_mState == mRUN)
		{
			vNEXTION_SendString("page1.t10", "RUN");
		}
		else if(pStateMachine->enum_mState == mFAULT)
		{
			vNEXTION_SendString("page1.t10", "FAULT");
		}
		
		if(pStateMachine->enum_ctrMode == OPEN_LOOP)
		{
			vNEXTION_SendString("page1.t8", "OPENLOOP");
			vNEXTION_SendString("page3.t0", "Va(%)");
			if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_IO)
			{
				vNEXTION_SendString("page3.bt1", "OPENLOOP");
				vNEXTION_SendVal("page3.bt1", 0, DATA_INT);	
			}
		}
		else if(pStateMachine->enum_ctrMode == TORQUE)
		{
			vNEXTION_SendString("page1.t8", "TORQUE");
			vNEXTION_SendString("page3.t0", "Spd_SP(%)");
			if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_IO)
			{
				vNEXTION_SendString("page3.bt1", "TORQUE");
				vNEXTION_SendVal("page3.bt1", 1, DATA_INT);
			}
		}
		else if(pStateMachine->enum_ctrMode == SPEED)
		{
			vNEXTION_SendString("page1.t8", "SPEED");
			vNEXTION_SendString("page3.t0", "Spd_SP(%)");
			if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_IO)
			{
				vNEXTION_SendString("page3.bt1", "SPEED");
				vNEXTION_SendVal("page3.bt1", 2, DATA_INT);
			}
		}
		
		if(pStateMachine->ui8_sysLOCAL_REMOTE == LOCAL)
		{
			vNEXTION_SendString("page1.t7", "LOCAL");
		}
		else if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_IO)
		{
			vNEXTION_SendString("page1.t7", "REMOTE");
		}
		
		if(pQEI->Direction == 0)				//Forward
		{
			vNEXTION_SendString("page1.t9", "FWD");
		}
		else
		{
			vNEXTION_SendString("page1.t9", "REV");
		}
		
		//HMI Display Control - Page 2
		vNEXTION_SendVal("page2.n0", pNEXVar->page2_n0_DI0, DATA_INT);
		vNEXTION_SendVal("page2.n1", pNEXVar->page2_n1_DI1, DATA_INT);
		vNEXTION_SendVal("page2.n2", pNEXVar->page2_n2_DI2, DATA_INT);
	}
}
/**********************************************************************************************/
void vNEXTION_RecvRetNumHandle(uint8_t RetNum)
{	
	
}
/**********************************************************************************************/
/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
void vNEXTION_SendCmd(char *cmd, uint8_t val)
{ 
	char buf[50];   
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
  int len = sprintf(buf, "%s=%d",cmd,val);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1000);
	HAL_UART_Transmit(&huart3, hmi_EndCmd, 3, 1000);
}
/**********************************************************************************************/
void vNEXTION_SendPCO(char *ID, int32_t Val)
{
	char buf[50];   
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
  int len = sprintf(buf, "%s.pco=%d",ID,Val);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1000);
	HAL_UART_Transmit(&huart3, hmi_EndCmd, 3, 1000);
}
/**********************************************************************************************/
void vNEXTION_SendVal(char *ID, int Val, uint8_t dataType)
{
	char buf[50], buf1[20];
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
	
	int len = sprintf(buf, "%s.val=%d", ID, Val);
	
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1000);				//Send data
	HAL_UART_Transmit(&huart3, hmi_EndCmd,3, 1000);								//Send End Command
	if(dataType == DATA_XFLOAT)	//x10 data
	{
		if(Val >= 0 && Val < 100)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 1);									//Number of Digit in the Left
			HAL_UART_Transmit(&huart3, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart3, hmi_EndCmd,3, 1000);
		}
		else if(100<= Val && Val < 1000)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 2);									//Number of Digit in the Left
			HAL_UART_Transmit(&huart3, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart3, hmi_EndCmd,3, 1000);
		}
		else if(Val >=1000 && Val < 10000)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 3);									//Number of Digit in the Left			
			HAL_UART_Transmit(&huart3, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart3, hmi_EndCmd,3, 1000);
		}
		else if(Val >=10000 && Val <100000)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 4);									//Number of Digit in the Left			
			HAL_UART_Transmit(&huart3, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart3, hmi_EndCmd,3, 1000);
		}
	}
}
/**********************************************************************************************/
void vNEXTION_SendString(char *ID, char *string)
{
	char buf[50];
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
	int len = sprintf(buf, "%s.txt=\"%s\"", ID, string);
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 1000);
	HAL_UART_Transmit(&huart3, hmi_EndCmd, 3, 100);
}
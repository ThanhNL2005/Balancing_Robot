#include "main.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1; //RS485
extern UART_HandleTypeDef huart5; //NEXTION HMI
extern UART_HandleTypeDef huart6; //ESP32
extern uint8_t ui8_Converter_Type;
//NEXTION HMI
extern NEXTION NEXVar;

/**********************************************************************************************/
void vNEXTION_Init(NEXTION *pNEXVar)
{
	pNEXVar->UpdateEN = 1;
	pNEXVar->ui16_GraphOffset = 80;
}
/***********************************Send data to NEXTION HMI**********************************/
void vNEXTION_DisplayHandle(adc_Objt *pADC, machine_Control *pStateMachine,	RAMP *pRAMP_Ia, RAMP *pRAMP_SpdSP, QEI *pQEI, machine_Parameter *pMParas, NEXTION *pNEXVar)
{
	float R = 0;
	static int cnt = 0; //For test purpose
	
	if(pNEXVar->UpdateEN == 1)
	{
		//Page number and Xfloat
		pNEXVar->page1_x0_IaFbx100 = pStateMachine->f_Iak*100;
		pNEXVar->page1_x4_RPMx100 = pStateMachine->f_GearShaft_QEI_Speed*100;
		pNEXVar->page1_x5_SpdSPx100 = pStateMachine->f_GearShaft_SpdSP*100;
		pNEXVar->page1_n0_SpeedEs = pStateMachine->f_GearShaft_EsSpeed;
		pNEXVar->page1_n2_Vdc = pADC->f_Vbus_Filter;
		
		if(ui8_Converter_Type == LOAD)
		{
			pNEXVar->page1_x1_IaSPx100 = pStateMachine->f_Ia_SP*100;
		}
		else if(ui8_Converter_Type == MOTOR)
		{
			pNEXVar->page1_n1_VaSP = pStateMachine->f_Va_SP;
		}
		//Display Ia on Gauge z0
		R = 270.0f/pMParas->f_Ia_max;
		if(pADC->f_Ia_Filter >= 0 && pADC->f_Ia_Filter < 45.0f/R)	
		{
			pNEXVar->page1_z0_IaFbx100 = 315 + pADC->f_Ia_Filter*R;
		}
		else if(pADC->f_Ia_Filter >= 45.0f/R)
		{
			pNEXVar->page1_z0_IaFbx100 = pADC->f_Ia_Filter*R - 45;
		}
		//Display rpm on Gauge z2
		R = 270.0f/pMParas->f_GearShaft_Spd_max;
		//Display encoder speed
		if(pStateMachine->f_MotorShaft_QEI_Speed >= 0 && pStateMachine->f_MotorShaft_QEI_Speed < 45.0f/R)	
		{
			pNEXVar->page1_z2_SpeedFB = 315 + pStateMachine->f_MotorShaft_QEI_Speed*R;
		}
		else if(pStateMachine->f_MotorShaft_QEI_Speed >= 45.0f/R)
		{
			pNEXVar->page1_z2_SpeedFB = pStateMachine->f_MotorShaft_QEI_Speed *R - 45;
		}
		//Display Vdc on Gauge z3
		R = 270.0f/pADC->f_MaxVbus;
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
		//Page 2
		pNEXVar->page2_n1_DI1 = HAL_GPIO_ReadPin(DI_1_GPIO_Port, DI_1_Pin);
		pNEXVar->page2_n2_DI2 = HAL_GPIO_ReadPin(DI_2_GPIO_Port, DI_2_Pin);

		pNEXVar->page2_n17_SP2 = pStateMachine->f_GearShaft_SpdSP;		//rpm
		pNEXVar->page2_n19_QEI_DIR = pQEI->i16t_Direction; 
		pNEXVar->page2_n20_QEI_CNT = pQEI->ui16t_Cnt_k;
		if(++cnt>999)
			cnt = 0;
		pNEXVar->page2_n21_MessCnt = cnt;
		
		//HMI Display Control - Page 1
		vNEXTION_SendVal("page1.x0", pNEXVar->page1_x0_IaFbx100, DATA_XFLOAT);
		vNEXTION_SendVal("page1.x2", pNEXVar->page1_x2_IExcFbx100, DATA_XFLOAT);
		vNEXTION_SendVal("page1.x4", pNEXVar->page1_x4_RPMx100, DATA_XFLOAT);
		vNEXTION_SendVal("page1.x5", pNEXVar->page1_x5_SpdSPx100, DATA_XFLOAT);
		vNEXTION_SendVal("page1.n2", pNEXVar->page1_n2_Vdc, DATA_INT);
		
		vNEXTION_SendVal("page1.z0", pNEXVar->page1_z0_IaFbx100, DATA_INT);
		vNEXTION_SendVal("page1.z1", pNEXVar->page1_z1_IExcFbx100, DATA_INT);
		vNEXTION_SendVal("page1.z2", pNEXVar->page1_z2_SpeedFB, DATA_INT);
		vNEXTION_SendVal("page1.z3", pNEXVar->page1_z3_Vdc, DATA_INT);
		
		if(ui8_Converter_Type == LOAD)
		{
			vNEXTION_SendVal("page1.x1", pNEXVar->page1_x1_IaSPx100, DATA_XFLOAT);	
			vNEXTION_SendVal("page1.x3", pNEXVar->page1_x3_IExcSPx100, DATA_XFLOAT);	
		}
		else if(ui8_Converter_Type == MOTOR)
		{
			vNEXTION_SendVal("page1.n1", pNEXVar->page1_n1_VaSP, DATA_INT);	
			vNEXTION_SendVal("page1.n3", pNEXVar->page1_n3_VfSP, DATA_INT);
		}
		
		if(pStateMachine->enum_mState == mSENSOR_CALIB)
		{
			vNEXTION_SendString("page1.t10", "CALIB");
		}
	
		else if(pStateMachine->enum_mState == mSTANDSTILL)
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
		
		if(pStateMachine->enum_ctrMode == OPEN_LOOP_VOL)
		{
			vNEXTION_SendString("page1.t8", "OPENLOOP");
		}
		else if(pStateMachine->enum_ctrMode == TORQUE_NO_ENCODER)
		{
			vNEXTION_SendString("page1.t8", "TRQ_WoQE");
		}
		else if(pStateMachine->enum_ctrMode == TORQUE_ENCODER)
		{
			vNEXTION_SendString("page1.t8", "TRQ_QE");
		}
		else if(pStateMachine->enum_ctrMode == SPEED_ENCODER)
		{
			vNEXTION_SendString("page1.t8", "SPD_QE");
		}
		else if(pStateMachine->enum_ctrMode == SPEED_SENSORLESS)
		{
			vNEXTION_SendString("page1.t8", "SPD_SS");
		}
		
		if(pStateMachine->ui8_sysLOCAL_REMOTE == LOCAL_HMI)
		{
			vNEXTION_SendString("page1.t7", "LOCAL");
		}
		else if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_IO)
		{
			vNEXTION_SendString("page1.t7", "RMT_IO");
		}
		else if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_MODBUS)
		{
			vNEXTION_SendString("page1.t7", "MODBUS");
		}
		else if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_CAN)
		{
			vNEXTION_SendString("page1.t7", "CAN");
		}
		else if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_WIFI)
		{
			vNEXTION_SendString("page1.t7", "WIFI");
		}
		else if(pStateMachine->ui8_sysLOCAL_REMOTE == REMOTE_LORA)
		{
			vNEXTION_SendString("page1.t7", "LORA");
		}
		
		if(pQEI->i16t_Direction == 0)				//Forward
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
		vNEXTION_SendVal("page2.n3", pNEXVar->page2_n3_DI3, DATA_INT);
		vNEXTION_SendVal("page2.n16", pNEXVar->page2_n16_FAULT, DATA_INT);
		
		vNEXTION_SendVal("page2.n12", pNEXVar->page2_n12_DB, DATA_INT);
		vNEXTION_SendVal("page2.n14", pNEXVar->page2_n14_RLY, DATA_INT);
		vNEXTION_SendVal("page2.n15", pNEXVar->page2_n15_BYPASS, DATA_INT);
		
		//vNEXTION_SendVal("page2.n17", pNEXVar->page2_n17_SP2, DATA_INT);
		//vNEXTION_SendVal("page2.n18", pNEXVar->page2_n18_RAMP2, DATA_INT);
		vNEXTION_SendVal("page2.n19", pNEXVar->page2_n19_QEI_DIR, DATA_INT);
		vNEXTION_SendVal("page2.n20", pNEXVar->page2_n20_QEI_CNT, DATA_INT);
		vNEXTION_SendVal("page2.n21", pNEXVar->page2_n21_MessCnt, DATA_INT);
		
		//HMI Display Control - Page 3, 4	
		if(ui8_Converter_Type == LOAD)
		{
			pNEXVar->page3_IaFbx10 = pStateMachine->f_Iak*10;
			pNEXVar->page3_IaRefx10 = pStateMachine->f_Ia_SP*10;
			pNEXVar->page3_IaRampx10 = pRAMP_Ia->f_Output*10;
			pNEXVar->page3_Ua = pStateMachine->f_Va_control;
			
			//Note - 40unit per div for NEXTION 2.4inch HMI
			vNEXTION_SendVal("page3.IaRefx10", pNEXVar->page3_IaRefx10*2 + pNEXVar->ui16_GraphOffset, DATA_INT);
			vNEXTION_SendVal("page3.IaFbx10", pNEXVar->page3_IaFbx10*2 + pNEXVar->ui16_GraphOffset, DATA_INT);
			vNEXTION_SendVal("page3.IaRampx10", pNEXVar->page3_IaRampx10*2 + pNEXVar->ui16_GraphOffset, DATA_INT);
			vNEXTION_SendVal("page3.Ua", pNEXVar->page3_Ua*3 + pNEXVar->ui16_GraphOffset, DATA_INT);
			
			vNEXTION_SendVal("page3.x0", pNEXVar->page3_IaRefx10*10, DATA_XFLOAT);
			vNEXTION_SendVal("page3.x2", pNEXVar->page3_IaFbx10*10, DATA_XFLOAT);
			vNEXTION_SendVal("page3.x1", pNEXVar->page3_IaRampx10*10, DATA_XFLOAT);
			vNEXTION_SendVal("page3.x3", pNEXVar->page1_x4_RPMx100, DATA_XFLOAT);
			vNEXTION_SendVal("page3.n0", pNEXVar->page3_Ua, DATA_INT);
		
			pNEXVar->page4_x0_RPM_SPx100 = pNEXVar->page1_x5_SpdSPx100;
			pNEXVar->page4_x1_RPM_RAMPx100 = pRAMP_SpdSP->f_Output*100;
			pNEXVar->page4_x2_RPM_FBx100 = pNEXVar->page1_x4_RPMx100;
			pNEXVar->page4_x3_Iax100 = pNEXVar->page1_x0_IaFbx100;
			
			vNEXTION_SendVal("page4.x0", pNEXVar->page4_x0_RPM_SPx100, DATA_XFLOAT);
			vNEXTION_SendVal("page4.x1", pNEXVar->page4_x1_RPM_RAMPx100, DATA_XFLOAT);
			vNEXTION_SendVal("page4.x2", pNEXVar->page4_x2_RPM_FBx100, DATA_XFLOAT);
			vNEXTION_SendVal("page4.x3", pNEXVar->page4_x3_Iax100, DATA_XFLOAT);
			
			vNEXTION_SendVal("page4.RPM_SP", 0.2f*pStateMachine->f_GearShaft_SpdSP + pNEXVar->ui16_GraphOffset, DATA_INT);
			vNEXTION_SendVal("page4.RPM_RAMP", 0.2f*pRAMP_SpdSP->f_Output + pNEXVar->ui16_GraphOffset, DATA_INT);
			vNEXTION_SendVal("page4.RPM_FB", 0.2f*pStateMachine->f_GearShaft_QEI_Speed + pNEXVar->ui16_GraphOffset, DATA_INT);
			vNEXTION_SendVal("page4.Ia", 0.2f*pNEXVar->page4_x3_Iax100 + pNEXVar->ui16_GraphOffset, DATA_INT);
		}
	}
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
	HAL_UART_Transmit(&huart5, (uint8_t *)buf, len, 1000);
	HAL_UART_Transmit(&huart5, hmi_EndCmd, 3, 1000);
}
/**********************************************************************************************/
void vNEXTION_SendPCO(char *ID, int32_t Val)
{
	char buf[50];   
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
  int len = sprintf(buf, "%s.pco=%d",ID,Val);
	HAL_UART_Transmit(&huart5, (uint8_t *)buf, len, 1000);
	HAL_UART_Transmit(&huart5, hmi_EndCmd, 3, 1000);
}
/**********************************************************************************************/
void vNEXTION_SendVal(char *ID, int Val, uint8_t dataType)
{
	char buf[50], buf1[20];
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
	
	int len = sprintf(buf, "%s.val=%d", ID, Val);
	
	HAL_UART_Transmit(&huart5, (uint8_t *)buf, len, 1000);				//Send data
	HAL_UART_Transmit(&huart5, hmi_EndCmd,3, 1000);								//Send End Command
	if(dataType == DATA_XFLOAT)	//x100 data
	{
		if(Val >= 0 && Val < 100)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 0);									//Number of Digit in the Left
			HAL_UART_Transmit(&huart5, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart5, hmi_EndCmd,3, 1000);
		}
		else if(100<= Val && Val < 1000)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 1);									//Number of Digit in the Left
			HAL_UART_Transmit(&huart5, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart5, hmi_EndCmd,3, 1000);
		}
		else if(Val >=1000 && Val < 10000)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 2);									//Number of Digit in the Left			
			HAL_UART_Transmit(&huart5, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart5, hmi_EndCmd,3, 1000);
		}
		else if(Val >=10000 && Val <100000)
		{
			len = sprintf(buf1, "%s.vvs0=%d", ID, 3);									//Number of Digit in the Left			
			HAL_UART_Transmit(&huart5, (uint8_t *)buf1, len, 1000);
			HAL_UART_Transmit(&huart5, hmi_EndCmd,3, 1000);
		}
	}
}
/**********************************************************************************************/
void vNEXTION_SendString(char *ID, char *string)
{
	char buf[50];
	uint8_t hmi_EndCmd[3] = {0xff, 0xff, 0xff};
	int len = sprintf(buf, "%s.txt=\"%s\"", ID, string);
	HAL_UART_Transmit(&huart5, (uint8_t *)buf, len, 1000);
	HAL_UART_Transmit(&huart5, hmi_EndCmd, 3, 100);
}

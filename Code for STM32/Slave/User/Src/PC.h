#include "stm32f1xx_hal.h"
#include "main.h"
/**********************************************************************************************/
typedef struct
{
	/*Page 1*/
	int page1_z0_IaFbx10, page1_z1_IExcFbx10, page1_z2_SpeedFB, page1_z3_Vdc;
	int page1_x0_IaFbx10, page1_x2_IExcFbx10, page1_n0_SpeedFB, page1_n2_Vdc;
	int page1_x1_IaSPx10, page1_x3_IExcSPx10, page1_n1_SpeedSP;
	
	char page1_t14_ChargeState[20];
	char page1_t8_ControlMode[20];
	char page1_t10_MachineState[20];
	char page1_t6_ErrorMess[50];
	char page1_t7_LOCAL_REMOTE[50];
	
	/*Page 2*/
	uint16_t page2_n0_DI0, page2_n1_DI1, page2_n2_DI2, page2_n3_DI3;
	uint16_t page2_n4_DI4, page2_n5_DI5, page2_n6_DI6, page2_n16_FAULT;
	uint16_t page2_n7_DO0, page2_n8_DO1, page2_n9_DO2, page2_n10_DO3, page2_n11_DO4;
	uint16_t page2_n12_DB, page2_n13_EN, page2_n14_RST, page2_n15_BYPASS;
	uint16_t page2_n17_SP2, page2_n18_RAMP2, page2_x0_SP1, page2_x1_RAMP1;
	uint16_t page2_n19_QEI_DIR, page2_n20_QEI_CNT;
	int page2_x2_LaxDia_dt;
	/*Page 3*/
	uint16_t page3_n0_PU_SPEED_SP, page3_n1_PU_Te_SP, page3_n2_PU_IF_SP;
	uint16_t page3_h0_PU_SPEED_SP, page3_h1_PU_Te_SP, page3_h2_PU_IF_SP;
	uint16_t page3_n3_PU_SPEED_FB, page3_n4_PU_Te_FB, page3_n5_PU_IF_FB;
	/*Data payload*/
	uint32_t messCnt, rxVal;
	uint8_t UpdateEN;
}NEXTION;

//NEXTION HMI interface
void vNEXTION_Init(NEXTION *pNEXVar);
void vNEXTION_SendVal(char *ID, int32_t Val, uint8_t dataType);
void vNEXTION_SendString(char *ID, char *string);
void vNEXTION_SendCmd(char *cmd, uint8_t val);
void vNEXTION_SendPCO(char *ID, int32_t val);
void vNEXTION_RecvRetNumHandle(uint8_t RetNum);
void vNEXTION_DisplayHandle(adc_Objt *pADC, machine_Control *pStateMachine, cPI *pSpeed, cPI *pIa,
	RAMP *pRAMP_Ia, RAMP *pRAMP_Speed, QEI *pQEI, machine_Parameter *pMParas, NEXTION *pNEXVar);
#include "stm32f1xx_hal.h"
#include "main.h"
/**********************************************************************************************/
typedef enum
{
	ACC, 
	RUN, 
	DEC
}RAMP_STATE;
/**********************************************************************************************/
//Ramp Fcn generator
typedef struct
{
	float SPk;												//Current and past value of setpoint
	float inputMax, inputMin;
	uint16_t T_Acc, T_Dec;						//Ramp up and down time (s)
	float delta_Acc, delta_Dec;				//step size in ACC and DEC mode
	uint16_t set_AccCnt, set_DecCnt;	//Set count - 10ms Resolution
	RAMP_STATE State;
	float Output;
}RAMP;
/**********************************************************************************************/
typedef enum
{
	mSTANDSTILL,
	mSTOP,  
	mRUN, 
	mFAULT
}machine_State;
/**********************************************************************************************/
typedef enum
{
	OPEN_LOOP,
	TORQUE, 
	SPEED
}control_Mode;
/**********************************************************************************************/
typedef enum
{
	CHARGE, 		//Charging the DC bus Capacitor via Resistor	
	DOL					//Direct OnLine
}charge_State;
/**********************************************************************************************/
typedef enum
{
	NORMAL, 		
	ALARM_L,
	ALARM_H,
	FAULT_L,
	FAULT_H, 
	EXT_FAULT
}OP_State;		//Operation state: NORMAL, ALARM, FAULT
/**********************************************************************************************/
//Machine control
typedef struct
{
	control_Mode ctrMode;				//cMode = SPEED or Torque
	machine_State mState;				//State = STOP, RUN, or FAULT
	charge_State chState;				//cState = CHARGE or DOL
	uint8_t sysENABLE, sysLOCAL_REMOTE;
	
	float If_H_Alarm, If_H_Fault;
	float If_L_Alarm, If_L_Fault;
	float Ia_H_Alarm, Ia_H_Fault;
	float Speed_H_Alarm, Speed_H_Fault;
	float Vdc_H_Alarm, Vdc_H_Fault , Vdc_L_Alarm, Vdc_L_Fault;
	
	OP_State If_State, Ia_State, Vdc_State, Speed_State, EXT_State;
	float Vdc_Nom;							//Nominal value of DC bus voltage
	float Vdc_Hi, Vdc_Lo;				//High and Low Level of the DC bus voltage, handle the charge state
	float Speed_SP, Te_SP, Ia_SP, If_SP, Vol_SP;
	int Dir;										//1: Forward, -1: Reverse
	int REVERSE_ON;							//1: REVERSE, 0: NORMAL
	int isrCnt;
	float EsSpeed;							//Estimation of speed
	float SpeedLimit, KLimit;		//Used to limit speed in Torque Mode
	float SpeedEs;							//SpeedEs is the output of saturation
	float Vcontrol;							//Final control signal which is send to PWM module
}machine_Control;
/**********************************************************************************************/
//Machine parameters
typedef struct
{
	float Ia_rated, If_rated, Va_rated;
	float Speed_rated, Speed_max;
	float La, Ra;
	float kPhiConst;
	float Power_rated, Torque_rated;
}machine_Parameter;
/**********************************************************************************************/
//Config the Ramp Fcn genertor
void vRamp_Config(RAMP *pRamp, uint16_t Tacc_Sec, uint16_t Tdec_Sec, float inputMin, float inputMax, float Ts);
float fRampFcnGen(float SP, RAMP *pRamp);														//Ramp Fcn generator
/*Safety chain handle*/
void vSafetyChainHandle(adc_Objt *pAdcObjt, machine_Control *pMControl, QEI *pQEI);
void vStateMachine(void);
void vStateMachineConfig(machine_Control *pMControl, machine_Parameter *pMParas);
void vMachineParameterConfig(machine_Parameter *pMParas);
void vDIO_Task(machine_Control *pMControl);
void vStateMachineLoop(void);																				//State machine
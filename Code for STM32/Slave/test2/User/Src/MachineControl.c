#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "main.h"
/**********************************************************************************************/
extern const float f_base_Ts;			//PWM isr frequency - 20kHz
extern const float f_Ia_Ts;				//Sampling frequency of the current control loop - 2kHz
extern const float f_Spd_Ts;			//Sampling frequency of the speed control loop - 50Hz
extern const float f_Gear_Ratio;	//Gear box reduced speed ratio of motor
extern const float f_Vbus_Nom;		//Norminal dc bus voltage - V
extern const uint8_t ui8_Converter_Type;
extern const uint8_t ui8_FirmwareLevel;	
extern float f_Ti_Spd; 						//second
extern float f_Ti_Ia;							//second
/*PWM*/
extern PWM PWM_Var;
//GPIO object
extern gpio_Objt DI_Objt[NumOfDI];
/*ADC data*/
extern adc_Objt adc_Objt_var;
extern ADC_HandleTypeDef hadc1;
extern volatile uint16_t ADC_CH1[6];
/*DAC data*/
extern DAC_OUT stDAC_Var;
extern DAC_HandleTypeDef hdac;
/*uart data*/
extern usartData usart4_Tx, usart4_Rx; 					//RS485
extern usartData usart5_Tx, usart5_Rx; 					//HMI
extern NEXTION NEXVar;
extern UART_HandleTypeDef huart4, huart5;
extern uint8_t First_Cycle_Flag;
//PI controller
extern cPI stPI_Speed, stPI_Ia;		
extern const float f_base_Ts;
//State machine
extern machine_Control stMControl_Var;		
extern machine_Parameter stMachine_Var;
//Setpoint
extern setpoint SP_Var;
//RAMP Generator (Ampere with Ia, and rpm with Speed)
extern RAMP stRamp_IaSP, stRamp_SpdSP, stRamp_VaSP;			
//Quadrature Encoder
extern QEI stQEI_Var;	
//Low pass filter
extern LPF st_LPF_AD_Ia_Offset;
extern LPF st_LPF_IaSP;
extern LPF st_LPF_Ia;
extern LPF st_LPF_dotIa;
extern LPF st_LPF_Vbus;
extern LPF st_LPF_SpdSP;
extern LPF st_LPF_EsSpd;
				char buffer[50];
// CAN
extern CAN_data CAN_1;
extern double N_Round;
extern uint8_t ID_CAN;
/**********************************************************************************************/
void vDIO_Task_Handle(machine_Control *pMControl)
{
	/*DI1 - Run/Stop command*/
	if(DI_Objt[1].event== Low && pMControl->ui8_sysENABLE==1&&pMControl->ui8_sysLOCAL_REMOTE==REMOTE_IO && stMControl_Var.enum_mState != mFLT)	//RUN
	{
		if(pMControl->enum_mState !=mFLT)
		{
			stMControl_Var.enum_mState = mRUN;
			//Enable the PWM module
			user_pwm_Enable(&PWM_Var);
		}
	}
	else if(DI_Objt[1].event== High && pMControl->ui8_sysENABLE==1&&pMControl->ui8_sysLOCAL_REMOTE==REMOTE_IO) //STOP
//		else if(DI_Objt[1].event== High && pMControl->ui8_sysENABLE==1)
	{
		if(pMControl->enum_mState == mRUN)
			pMControl->enum_mState = mSTOP;
	}
	/*DI2 - Direction select*/
	if(DI_Objt[2].event == High)	//Direction = Forward
	{
		pMControl->i_Dir = FORWARD;
	}
	
	if(DI_Objt[2].event == Low)		//Direction = Reverse
	{
		pMControl->i_Dir = REVERSE;
	}
}
/**********************************************************************************************/
void vMachineParameterConfig(machine_Parameter *pMParas)
{
	/*Motor parameters*/
	pMParas->f_Ia_rated = 3.6f;											//Ampere
	pMParas->f_Va_rated = 24.0f;										//V
	pMParas->f_Motor_Spd_rated = 9000.0f;						//rpm
	pMParas->f_Motor_Torque_rated = 0.093163175; 		//Nm
	pMParas->f_Power_rated = (pMParas->f_Motor_Torque_rated*pMParas->f_Motor_Spd_rated)/9.55f;	//W
	pMParas->f_kPhiConst = pMParas->f_Motor_Torque_rated/pMParas->f_Ia_rated;
	pMParas->f_Gear_Ratio = f_Gear_Ratio;								//reduced speed gearbox
	pMParas->f_GearShaft_Spd_rated = pMParas->f_Motor_Spd_rated/pMParas->f_Gear_Ratio;					//rpm
	pMParas->f_GearShaft_Torque_rated = pMParas->f_Motor_Torque_rated*pMParas->f_Gear_Ratio;		//Nm
	/*Amature resistance estimation*/
	//pMParas->f_Ra = (pMParas->f_Va_rated - (pMParas->f_kPhiConst*pMParas->f_Motor_Spd_rated)/9.55f)/pMParas->f_Ia_rated;
	pMParas->f_Ra = 1.4f;
	pMParas->f_La = 0.01f;							//20mH
	/*Protection setup*/
	pMParas->f_Overload_Factor = 1.5f;
	pMParas->f_OverSpeed_Factor = 1.0f;			
	pMParas->f_Ia_max = pMParas->f_Ia_rated*pMParas->f_Overload_Factor;
	pMParas->f_Motor_Spd_max = pMParas->f_Motor_Spd_rated*pMParas->f_OverSpeed_Factor; 	//Motor shaft rpm
	pMParas->f_GearShaft_Spd_max = pMParas->f_Motor_Spd_max/pMParas->f_Gear_Ratio;
	pMParas->f_Motor_Torque_max = pMParas->f_Ia_max*pMParas->f_kPhiConst;
	pMParas->f_GearShaft_Torque_Max = pMParas->f_GearShaft_Torque_rated*pMParas->f_Overload_Factor;
}
/**********************************************************************************************/
void setpointInit(setpoint *pSPVar, machine_Parameter *pMParas)
{	
	/*Setpoint from Analog inputs*/
	pSPVar->f_PU_AN_SpdSP = 0.0f;
	pSPVar->f_PU_AN_IaSP = 0.0f;
	pSPVar->f_PU_AN_VaSP = 0.0f;	
	
	pSPVar->f_SI_AN_MotorShaft_SpdSP = 0.0f; 	
	pSPVar->f_SI_AN_GearShaft_SpdSP = 300.0f;	
	pSPVar->f_SI_AN_IaSP = 2.0f;																												
	pSPVar->f_SI_AN_VaSP = 0.0f;  																						
	
	/*Setpoint via Modbus communication port*/
	pSPVar->f_PU_Modbus_SpdSP = 0.0f; 
	pSPVar->f_PU_Modbus_IaSP = 0.0f;	
	pSPVar->f_PU_Modbus_VaSP = 0.0f; 	
	pSPVar->f_SI_Modbus_MotorShaft_SpdSP = 0.0f; 	
	pSPVar->f_SI_Modbus_GearShaft_SpdSP = 0.0f; 	
	pSPVar->f_SI_Modbus_IaSP = 0.0f;																								
	pSPVar->f_SI_Modbus_VaSP = 0.0f;  

	/*Setpoint via CAN communication port*/
	pSPVar->f_PU_CAN_SpdSP = 0.0f; 
	pSPVar->f_PU_CAN_IaSP = 0.0f;	
	pSPVar->f_PU_CAN_VaSP = 0.0f; 	
	pSPVar->f_SI_CAN_MotorShaft_SpdSP = 0.0f; 	
	pSPVar->f_SI_CAN_GearShaft_SpdSP = 0.0f; 	
	pSPVar->f_SI_CAN_IaSP = 0.0f;																								
	pSPVar->f_SI_CAN_VaSP = 0.0f; 

	/*Setpoint via PC-UART communication port*/
	pSPVar->f_PU_PC_UART_SpdSP = 0.0f; 
	pSPVar->f_PU_PC_UART_IaSP = 0.0f;	
	pSPVar->f_PU_PC_UART_VaSP = 0.0f; 	
	pSPVar->f_SI_PC_UART_MotorShaft_SpdSP = 0.0f; 		
	pSPVar->f_SI_PC_UART_GearShaft_SpdSP = 0.0f; 	
	pSPVar->f_SI_PC_UART_IaSP = 0.0f;																								
	pSPVar->f_SI_PC_UART_VaSP = 0.0f; 
}
/**********************************************************************************************/
void vSetPointGet(void)
{
	switch(stMControl_Var.ui8_sysLOCAL_REMOTE)
	{
		case REMOTE_IO:
		{
			stMControl_Var.f_Ia_SP = stMControl_Var.i_Dir*SP_Var.f_SI_AN_IaSP;							//Ampere
			stMControl_Var.f_Va_SP = stMControl_Var.i_Dir*SP_Var.f_SI_AN_VaSP;							//Vol
			//stMControl_Var.f_Va_SP = stMControl_Var.i_Dir*50;
			//  stMControl_Var.f_GearShaft_SpdSP = stMControl_Var.i_Dir*SP_Var.f_SI_AN_GearShaft_SpdSP;	//rpm
						stMControl_Var.f_Ia_SP = stMControl_Var.i_Dir*0.5;							//Ampere
//			stMControl_Var.f_Va_SP = stMControl_Var.i_Dir*SP_Var.f_SI_AN_VaSP;							//Vol
			stMControl_Var.f_GearShaft_SpdSP = stMControl_Var.i_Dir*400;
			break;
		}
		case REMOTE_PC_UART:
		{
			break;
		}case REMOTE_MODBUS:
		{
			break;
		}case REMOTE_CAN:
		{
			stMControl_Var.f_Ia_SP= CAN_1.IaSP/(stMachine_Var.f_kPhiConst*f_Gear_Ratio);//stMControl_Var.i_Dir*0.8;//CAN_IaSP;							//Ampere
			stMControl_Var.f_Va_SP = stMControl_Var.i_Dir*SP_Var.f_SI_AN_VaSP;
			stMControl_Var.f_GearShaft_SpdSP = stMControl_Var.i_Dir*25;
			break;
		}
	}
}
/**********************************************************************************************/
void vStateMachineConfig(machine_Control *pMControl, machine_Parameter *pMParas)
{
	pMControl->f_Ia_H_Alarm = pMParas->f_Ia_rated * 2.0f;
	pMControl->f_Ia_H_Fault = pMParas->f_Ia_rated *2.5f;
	
	pMControl->i_Speed_H_Alarm = pMParas->f_Motor_Spd_rated * 1.1f;
	pMControl->i_Speed_H_Fault = pMParas->f_Motor_Spd_rated * 1.2f;
	
	pMControl->f_Vdc_Nom = (uint16_t)(f_Vbus_Nom);								//Nominal value of DC bus voltage
	pMControl->ui16_Vdc_H_Alarm = pMControl->f_Vdc_Nom*1.1f;
	pMControl->ui16_Vdc_H_Fault = pMControl->f_Vdc_Nom*1.3f;
	pMControl->ui16_Vdc_L_Alarm = pMControl->f_Vdc_Nom*0.9f;
	pMControl->ui16_Vdc_L_Fault = pMControl->f_Vdc_Nom*0.7f;
	pMControl->ui16_Vdc_Hi = pMControl->f_Vdc_Nom*0.9f;
	pMControl->ui16_Vdc_Lo = pMControl->f_Vdc_Nom*0.8f;
	
	pMControl->ui8_sysENABLE = 1;											//Always ENABLE
	pMControl->ui8_sysLOCAL_REMOTE = REMOTE_CAN;		//REMOTE_IO||REMOTE_CAN		
	pMControl->enum_mState = mSENSOR_CALIB;						//Offset calib at the beginning
	pMControl->i_Dir = 1;															//FORWARD
	pMControl->enum_ctrMode = TORQUE_ENCODER; //SPEED_ENCODER||TORQUE_NO_ENCODER||TORQUE_ENCODER;
	pMControl->f_MotorShaft_EsSpeed = 0.0f;
	pMControl->f_GearShaft_EsSpeed = 0.0f;
	pMControl->f_EsSpeed_Kfactor = 1.0;
	pMControl->f_GearShaft_SpeedLimit = 1*pMParas->f_GearShaft_Spd_rated;	//Limit gear shaft speed in Torque control mode
	pMControl->f_Speed_SatErr = 0.0f;
	pMControl->f_Iak = 0.0f; pMControl->f_Iak_1 = 0.0f; 
	pMControl->f_dot_Ia = 0.0f; pMControl->f_dot_Ia_Filter = 0.0f;
	pMControl->f_EsSpeed_Gain = (pMControl->f_EsSpeed_Kfactor*9.55f/pMParas->f_kPhiConst);
	pMControl->f_BackCalGain = 0.5f;
	pMControl->ui8_BEMF_Compensate_EN = ENABLE; //DISABLE||ENABLE
}
/******Closed-loop amature current control for Load side - setpoint from analog input*******/
void vAmatureCurrentControl(void)
{
	if(stMControl_Var.enum_ctrMode == TORQUE_NO_ENCODER)
	{
		//Generate reference Amature current
		fRampFcnGen(stMControl_Var.f_Ia_SP, &stRamp_IaSP);

		//Run the PI Torque current controller
		fPI_Run(&stPI_Ia, stRamp_IaSP.f_Output, stMControl_Var.f_Iak);		
		//Compute the speed saturation error - used in speed limmit mode
		if(stMControl_Var.f_GearShaft_EsSpeed > stMControl_Var.f_GearShaft_SpeedLimit)
		{
			stMControl_Var.f_Speed_SatErr = stMControl_Var.f_GearShaft_EsSpeed - stMControl_Var.f_GearShaft_SpeedLimit;
		}
		else if(stMControl_Var.f_GearShaft_EsSpeed < -stMControl_Var.f_GearShaft_SpeedLimit)
		{
			stMControl_Var.f_Speed_SatErr = stMControl_Var.f_GearShaft_EsSpeed + stMControl_Var.f_GearShaft_SpeedLimit;
		}
		else
		{
			stMControl_Var.f_Speed_SatErr = 0.0f;
		}
		stMControl_Var.f_BEMF = stMachine_Var.f_kPhiConst*stMControl_Var.f_MotorShaft_EsSpeed/9.55f;
		//without the back EMF compensation
		if(stMControl_Var.ui8_BEMF_Compensate_EN == DISABLE)
		{
			stMControl_Var.f_Va_control = -(stPI_Ia.Usk - stMControl_Var.f_BackCalGain*stMControl_Var.f_Speed_SatErr);
		}
		else //Compensate the back EMF
		{
			stMControl_Var.f_Va_control = -(stPI_Ia.Usk - stMControl_Var.f_BackCalGain*stMControl_Var.f_Speed_SatErr + stMControl_Var.f_BEMF);
		}
	}		
	else if(stMControl_Var.enum_ctrMode == TORQUE_ENCODER)
	{
		//Generate reference Amature current
		//fRampFcnGen(stMControl_Var.f_Ia_SP, &stRamp_IaSP);
		//Run the PI Torque current controller
		fPI_Run(&stPI_Ia, stMControl_Var.f_Ia_SP, stMControl_Var.f_Iak);
		//Compute the speed saturation error - used in speed limmit mode
		if(stMControl_Var.f_GearShaft_QEI_Speed > stMControl_Var.f_GearShaft_SpeedLimit)
		{
			
			stMControl_Var.f_Speed_SatErr = stMControl_Var.f_GearShaft_QEI_Speed - stMControl_Var.f_GearShaft_SpeedLimit;
		}
		else if(stMControl_Var.f_GearShaft_QEI_Speed < -stMControl_Var.f_GearShaft_SpeedLimit)
		{
			stMControl_Var.f_Speed_SatErr = stMControl_Var.f_GearShaft_QEI_Speed + stMControl_Var.f_GearShaft_SpeedLimit;
		}
		else
		{
			stMControl_Var.f_Speed_SatErr = 0.0f;
		}
		stMControl_Var.f_BEMF = stMachine_Var.f_kPhiConst*stMControl_Var.f_GearShaft_QEI_Speed/9.55f;
		//without the back EMF compensation
		if(stMControl_Var.ui8_BEMF_Compensate_EN == DISABLE)
		{
			stMControl_Var.f_Va_control = -stPI_Ia.Usk;
//			stMControl_Var.f_Va_control = -(stPI_Ia.Usk - stMControl_Var.f_BackCalGain*stMControl_Var.f_Speed_SatErr);
		}
		else //Compensate the back EMF
		{
			stMControl_Var.f_Va_control = -(stPI_Ia.Usk + stMControl_Var.f_BEMF);
			//stMControl_Var.f_Va_control = stPI_Ia.Usk - stMControl_Var.f_BackCalGain*stMControl_Var.f_Speed_SatErr + stMControl_Var.f_BEMF;
		}
	}
	else if(stMControl_Var.enum_ctrMode == SPEED_SENSORLESS || stMControl_Var.enum_ctrMode == SPEED_ENCODER)
	{
		//Run the PI Torque current controller - Setpoint taking from the PI speed controller's output
		fPI_Run(&stPI_Ia, stPI_Speed.Usk, stMControl_Var.f_Iak);
		stMControl_Var.f_Va_control = -stPI_Ia.Usk;  // dièu chinh dau usk
	}
	//Set output voltage by using PWM module
	if(ui8_FirmwareLevel == TEST_MODE)	
		user_pwm_setvalue(&PWM_Var, 12.0f,f_Vbus_Nom, Unipolar); //TEST MODE Only
	else if(ui8_FirmwareLevel == RUN_MODE)
		user_pwm_setvalue(&PWM_Var, stMControl_Var.f_Va_control,adc_Objt_var.f_Vbus_Filter, Unipolar);
}
/*******Amature voltage control for Load side - setpoint from analog input*******/
void vAmatureVolControl(void)
{
	//Generate reference Amature voltage
	fRampFcnGen(stMControl_Var.f_Va_SP, &stRamp_VaSP);
	//Send reference Voltage to the power converter via PWM module
	stMControl_Var.f_Va_control = stRamp_VaSP.f_Output;
	if(ui8_FirmwareLevel == TEST_MODE)	
		user_pwm_setvalue(&PWM_Var, 12.0f,f_Vbus_Nom, Unipolar); //TEST MODE Only
	else if(ui8_FirmwareLevel == RUN_MODE)
	{
		user_pwm_setvalue(&PWM_Var, stMControl_Var.f_Va_control,adc_Objt_var.f_Vbus_Filter, Unipolar);
	}
}
/**********************************************************************************************/
void vStateMachine(void) //Machine control - Every 50us
{
	static uint16_t ui16_isrcnt_SensCalib = 0;
	static uint16_t ui16_isrcnt_50ms = 0;
	static uint16_t ui16_50msEvent = 0;	
	static uint16_t ui16_isrcnt_100us = 0;
	static uint16_t ui16_100usEvent = 0;	
	float f_ES_Speed; 
	
//	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);	//For Debug Only
	//sprintf(buffer, "Gia tri: %.2f\r\n", stMControl_Var.f_Iak);
	//HAL_UART_Transmit(&huart4, (uint8_t *)buffer,strlen(buffer), HAL_MAX_DELAY);
	//Handle the isr counter
	if(++ui16_isrcnt_50ms > 999) //50ms - used for gear shaft speed control loop
	{
		ui16_isrcnt_50ms = 0;
		ui16_50msEvent = 1;
	}
	if(++ui16_isrcnt_100us > 1) //250us - used for torque current control loop
	{
		ui16_isrcnt_100us = 0;
		ui16_100usEvent = 1;
	}
	/*-----------------------------------50us Tasks-----------------------------------*/
	/*Calib sensor offset - in 1000 sampling cycle*/
	if(stMControl_Var.enum_mState == mSENSOR_CALIB) 
	{
		//ADC read	
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_CH1, 6);
		//ADC Data Handle
		adc_Objt_var.ui16_AD_Ia_Offset = fLPF_Run(&st_LPF_AD_Ia_Offset, ADC_CH1[3]);
		if(++ui16_isrcnt_SensCalib > 999)
		{
			ui16_isrcnt_SensCalib = 0;
//			stMControl_Var.enum_mState = mSTANDSTILL;
			stMControl_Var.enum_mState = mRUN;
		}
	}	
	/*ADC data collect*/
	else
	{
		/*ADC read*/	
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_CH1, 6);
		//ADC Data Handle 
		vAdc_Data_Handle(&adc_Objt_var, &stMachine_Var, &SP_Var);
		/*dia/dt compute*/
		stMControl_Var.f_Iak = adc_Objt_var.f_Ia_Filter;
		stMControl_Var.f_dot_Ia = (stMControl_Var.f_Iak - stMControl_Var.f_Iak_1)/f_base_Ts;
		stMControl_Var.f_Iak_1 = stMControl_Var.f_Iak;
		
		/*---------------------------Setlect the Setpoint from corresponding channel---------------------------*/
		if(stMControl_Var.enum_mState == mSTOP)
		{
			stMControl_Var.f_Va_SP = 0.0f;							
			stMControl_Var.f_Ia_SP = 0.0f;							//Ampere
			stMControl_Var.f_GearShaft_SpdSP = 0.0f;
		}
		else
		{
			vSetPointGet();				
		}	
	}
	/*-------------------------Torque current control - 100us --------------------------*/
	if(ui16_100usEvent == 1)
	{
		
//		vDAC_CH1_IaWrite(&stDAC_Var, 1, stMControl_Var.f_Iak);
//		vDAC_CH1_IaWrite(&stDAC_Var, 2, stMControl_Var.f_Ia_SP);
		if(stMControl_Var.enum_mState == mSTOP || stMControl_Var.enum_mState == mRUN)
		{
			if(stMControl_Var.enum_ctrMode != OPEN_LOOP_VOL) 
			{
				//closed-loop amature current control - setpoint taken from Analog Input
				vAmatureCurrentControl();						
			}
			else
			{
				//Do nothing
			}
			if(stMControl_Var.enum_ctrMode == TORQUE_ENCODER || stMControl_Var.enum_ctrMode == TORQUE_NO_ENCODER)
			{
				if(stMControl_Var.enum_mState == mSTOP && stRamp_IaSP.f_Output == 0.0f && (fabs)(stMControl_Var.f_Iak) < 0.25f) //Dead zone = +-0.25A
				{
						stMControl_Var.enum_mState = mSTANDSTILL;
				}	
			}
		}
		else if(stMControl_Var.enum_mState == mSTANDSTILL)	
		{
			//Disable the PWM module
			user_pwm_Disable(&PWM_Var);
			user_pwm_setvalue(&PWM_Var, 0.0f,f_Vbus_Nom, Unipolar);	
			stMControl_Var.f_MotorShaft_EsSpeed = 0;
			stMControl_Var.f_GearShaft_EsSpeed = 0;
			//Config PI Speed Controller 20ms sampling time (50Hz)
			vPI_Init(&stPI_Speed, f_Ti_Spd, 0.3f, f_Spd_Ts, stMachine_Var.f_GearShaft_Spd_rated, stMachine_Var.f_Ia_max);
			//Config PI amature current controller with 0.5ms sampling time (2kHz)
			vPI_Init(&stPI_Ia, f_Ti_Ia, 0.3f, f_Ia_Ts, stMachine_Var.f_Ia_max, stMachine_Var.f_Va_rated);			
			//Config Quadrature Encoder
			vQEI_Config(&stQEI_Var,12, 4, f_Gear_Ratio, f_Spd_Ts);	
			//Config Ramp function generator
			vRamp_Config(&stRamp_SpdSP, 1, 1, 0, stMachine_Var.f_GearShaft_Spd_rated, f_Spd_Ts);
			vRamp_Config(&stRamp_VaSP, 5, 5, 0, stMachine_Var.f_Va_rated, f_Spd_Ts);
			vRamp_Config(&stRamp_IaSP, 1, 1, 0, stMachine_Var.f_Ia_rated, f_Ia_Ts);	
			stMControl_Var.f_Va_control = 0;			
		}
		else if(stMControl_Var.enum_mState == mFLT)
		{
			//Disable the PWM module
			user_pwm_Disable(&PWM_Var);
			user_pwm_setvalue(&PWM_Var, 0.0f,f_Vbus_Nom, Unipolar);	
			stMControl_Var.f_MotorShaft_EsSpeed = 0;
			stMControl_Var.f_GearShaft_EsSpeed = 0;
			//Config PI Speed Controller 20ms sampling time (50Hz)
			vPI_Init(&stPI_Speed, f_Ti_Spd, 0.3f, f_Spd_Ts, stMachine_Var.f_GearShaft_Spd_rated, stMachine_Var.f_Ia_max);
			//Config PI amature current controller with 0.5ms sampling time (2kHz)
			vPI_Init(&stPI_Ia, f_Ti_Ia, 0.3f, f_Ia_Ts, stMachine_Var.f_Ia_max, stMachine_Var.f_Va_rated);			
			//Config Quadrature Encoder
			vQEI_Config(&stQEI_Var, 12, 4, f_Gear_Ratio, f_Spd_Ts);	
			//Config Ramp function generator
			vRamp_Config(&stRamp_SpdSP, 1, 1, 0, stMachine_Var.f_GearShaft_Spd_rated, f_Spd_Ts);
			vRamp_Config(&stRamp_VaSP, 5, 5, 0, stMachine_Var.f_Va_rated, f_Spd_Ts);
			vRamp_Config(&stRamp_IaSP, 1, 1, 0, stMachine_Var.f_Ia_rated, f_Ia_Ts);		
			stMControl_Var.f_Va_control = 0;
		}
		ui16_100usEvent = 0;		
	}
	/*---------------------------------50ms Tasks------------------------------- */
	if(ui16_50msEvent == 1)
	{
		/*-----------------------------------------Clear 50ms event flag------------------------------------------*/
		ui16_50msEvent = 0;
		/*---------------------------Estimate the speed via back EMF/Encoder - NOT in STANDSTILL MODE---------------------------*/
		if(stMControl_Var.enum_mState != mSTANDSTILL && stMControl_Var.enum_mState != mFLT)
		{
			//Measure speed by encoder
			vRPM_Cal(&stQEI_Var, &stMachine_Var);
			stMControl_Var.f_MotorShaft_QEI_Speed = stQEI_Var.f_MotorShaft_rpm;
			stMControl_Var.f_GearShaft_QEI_Speed = stQEI_Var.f_GearShaft_rpm;
			
			//Estimate the motor shaft speed - sensorless
//			f_ES_Speed = stMControl_Var.f_EsSpeed_Gain*(stMControl_Var.f_Va_control - stMachine_Var.f_Ra*stMControl_Var.f_Iak);
//			stMControl_Var.f_MotorShaft_EsSpeed = fLPF_Run(&st_LPF_EsSpd, f_ES_Speed);
//			stMControl_Var.f_GearShaft_EsSpeed = stMControl_Var.f_MotorShaft_EsSpeed/stMachine_Var.f_Gear_Ratio;
		}		
		//Motor Run
		if(stMControl_Var.enum_mState == mSTOP || stMControl_Var.enum_mState == mRUN)	
		{
			if(stMControl_Var.enum_ctrMode == OPEN_LOOP_VOL)
			{
				//Amature voltage control
				vAmatureVolControl();
				if(stMControl_Var.enum_mState == mSTOP && stRamp_VaSP.f_Output == 0.0f)
				{
						stMControl_Var.enum_mState = mSTANDSTILL;
				}
			}			
			else if(stMControl_Var.enum_ctrMode == SPEED_SENSORLESS)
			{
				//Generate reference speed
				fRampFcnGen(stMControl_Var.f_GearShaft_SpdSP, &stRamp_SpdSP);
				//Run the PI Speed controller - Output of the speed controller is the reference amature current(Ampere)
				fPI_Run(&stPI_Speed, stRamp_SpdSP.f_Output, stMControl_Var.f_GearShaft_EsSpeed); 
				if(stMControl_Var.enum_mState == mSTOP && stRamp_SpdSP.f_Output == 0.0f)
				{
						stMControl_Var.enum_mState = mSTANDSTILL;
				}
			}
			else if(stMControl_Var.enum_ctrMode == SPEED_ENCODER)
			{
				//Generate reference speed
				fRampFcnGen(stMControl_Var.f_GearShaft_SpdSP, &stRamp_SpdSP);

				//Run the PI Speed controller - Output of the speed controller is the reference amature current(Ampere)
				fPI_Run(&stPI_Speed, stRamp_SpdSP.f_Output, stMControl_Var.f_GearShaft_QEI_Speed); 
				if(stMControl_Var.enum_mState == mSTOP && stRamp_SpdSP.f_Output == 0.0f)
				{
						stMControl_Var.enum_mState = mSTANDSTILL;
				}
			}
		}
	}
//	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);	//For Debug Only	
}
/**********************************************************************************************/
/*Safety chain handle*/
void vSafetyChainHandle(adc_Objt *pAdcObjt, machine_Control *pMControl, QEI *pQEI)
{
	//High amature current detect
	if(pMControl->enum_Ia_State != FAULT_H)
	{
		if((fabs)(pAdcObjt->f_Ia_Filter) > pMControl->f_Ia_H_Alarm && (fabs)(pAdcObjt->f_Ia_Filter) < pMControl->f_Ia_H_Fault)	//Alarm
		{
			pMControl->enum_Ia_State = ALARM_H;
		}
		else if((fabs)(pAdcObjt->f_Ia_Filter) > pMControl->f_Ia_H_Fault)		//Over current
		{
			stMControl_Var.enum_mState = mSTOP;
			pMControl->enum_Ia_State  = FAULT_H;															//Ia Over Current
		}
		else if((fabs)(pAdcObjt->f_Ia_Filter) < pMControl->f_Ia_H_Alarm*0.95f)
		{
			pMControl->enum_Ia_State = NORMAL;
		}
	}
	
	//Low VDC detect - Only in DOL mode
	if(pAdcObjt->f_Vbus_Filter < pMControl->ui16_Vdc_L_Alarm && pAdcObjt->f_Vbus_Filter > pMControl->ui16_Vdc_L_Fault)
	{
		pMControl->enum_Vdc_State = ALARM_L;
	}
	else if(pAdcObjt->f_Vbus_Filter <= pMControl->ui16_Vdc_L_Fault)
	{
		pMControl->enum_mState = mFLT;
		pMControl->enum_Vdc_State = FAULT_L;		//Low dc voltage
	}
	else if(pAdcObjt->f_Vbus_Filter > pMControl->ui16_Vdc_L_Alarm*1.05f)
	{
		pMControl->enum_Vdc_State = NORMAL;
	}
	
	//High VDC detect
	if(pAdcObjt->f_Vbus_Filter > pMControl->ui16_Vdc_H_Alarm && pAdcObjt->f_Vbus_Filter < pMControl->ui16_Vdc_H_Fault)
	{
		pMControl->enum_Vdc_State = ALARM_H;
	}
	else if(pAdcObjt->f_Vbus_Filter >= pMControl->ui16_Vdc_H_Fault)
	{
		stMControl_Var.enum_mState = mFLT;
		stMControl_Var.enum_Vdc_State = FAULT_H;			//Over voltage
	}
	else if(pAdcObjt->f_Vbus_Filter < pMControl->ui16_Vdc_H_Alarm*0.95f)
	{
		stMControl_Var.enum_Vdc_State = NORMAL;
	}
	
	//High speed detect
	if(stMControl_Var.f_GearShaft_EsSpeed > pMControl->i_Speed_H_Alarm && stMControl_Var.f_GearShaft_EsSpeed   < pMControl->i_Speed_H_Fault)
	{
		pMControl->enum_Speed_State = ALARM_H;
	}
	else if(stMControl_Var.f_GearShaft_EsSpeed >= pMControl->i_Speed_H_Fault)
	{
		pMControl->enum_mState = mFLT;
		pMControl->enum_Speed_State = FAULT_H;				//Over speed
	}
	else if(stMControl_Var.f_GearShaft_EsSpeed  < pMControl->i_Speed_H_Alarm*0.95f)
	{
		pMControl->enum_Speed_State = NORMAL;
	}
}
void Torque_control(void){
	static uint16_t ui16_isrcnt_SensCalib = 0;
	static uint16_t ui16_isrcnt_50ms = 0;
	static uint16_t ui16_50msEvent = 0;	
	static uint16_t ui16_isrcnt_100us = 0;
	static uint16_t ui16_100usEvent = 0;	
	float f_ES_Speed; 	
	

	if(++ui16_isrcnt_100us > 1) //250us - used for torque current control loop 10kHz
	{
		ui16_isrcnt_100us = 0;
		ui16_100usEvent = 1;
	}
		/*-----------------------------------50us Tasks-----------------------------------*/
	/*Calib sensor offset - in 1000 sampling cycle*/
	if(stMControl_Var.enum_mState == mSENSOR_CALIB) 
	{
		//ADC read	
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_CH1, 6);
		//ADC Data Handle
		adc_Objt_var.ui16_AD_Ia_Offset = fLPF_Run(&st_LPF_AD_Ia_Offset, ADC_CH1[3]);
		if(++ui16_isrcnt_SensCalib > 999)
		{
			ui16_isrcnt_SensCalib = 0;
			stMControl_Var.enum_mState = mSTANDSTILL;
			//stMControl_Var.enum_mState = mRUN;
		}
	}	
	/*ADC data collect*/
	else
	{
		/*ADC read*/	
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_CH1, 6);
		//ADC Data Handle
		vAdc_Data_Handle(&adc_Objt_var, &stMachine_Var, &SP_Var);
		/*dia/dt compute*/
		stMControl_Var.f_Iak = adc_Objt_var.f_Ia_Filter;
		stMControl_Var.f_dot_Ia = (stMControl_Var.f_Iak - stMControl_Var.f_Iak_1)/f_base_Ts;
		stMControl_Var.f_Iak_1 = stMControl_Var.f_Iak;
		
		/*---------------------------Setlect the Setpoint from corresponding channel---------------------------*/
		if(stMControl_Var.enum_mState == mSTOP)
		{
			stMControl_Var.f_Va_SP = 0.0f;							
			stMControl_Var.f_Ia_SP = 0.0f;							//Ampere
			stMControl_Var.f_GearShaft_SpdSP = 0.0f;
		}
		else
		{
			vSetPointGet();				
		}	
	}
	/****************************************************************************/
	if(ui16_100usEvent == 1){
		ui16_100usEvent = 0;
		if(++ui16_isrcnt_50ms > 24) 
	{
		ui16_isrcnt_50ms = 0;
		ui16_50msEvent = 1;
	}
						//Generate reference Amature current
		//fRampFcnGen(stMControl_Var.f_Ia_SP, &stRamp_IaSP);
		//Run the PI Torque current controller
		fPI_Run(&stPI_Ia, stMControl_Var.f_Ia_SP, stMControl_Var.f_Iak);
		//Compute the speed saturation error - used in speed limmit mode
		if(stMControl_Var.f_GearShaft_QEI_Speed > stMControl_Var.f_GearShaft_SpeedLimit)
		{
			
			stMControl_Var.f_Speed_SatErr = stMControl_Var.f_GearShaft_QEI_Speed - stMControl_Var.f_GearShaft_SpeedLimit;
		}
		else if(stMControl_Var.f_GearShaft_QEI_Speed < -stMControl_Var.f_GearShaft_SpeedLimit)
		{
			stMControl_Var.f_Speed_SatErr = stMControl_Var.f_GearShaft_QEI_Speed + stMControl_Var.f_GearShaft_SpeedLimit;
		}
		else
		{
			stMControl_Var.f_Speed_SatErr = 0.0f;
		}
		stMControl_Var.f_BEMF = stMachine_Var.f_kPhiConst*stMControl_Var.f_GearShaft_QEI_Speed/9.55f;
		//without the back EMF compensation
		if(stMControl_Var.ui8_BEMF_Compensate_EN == DISABLE)
		{
			stMControl_Var.f_Va_control = -stPI_Ia.Usk;
//			stMControl_Var.f_Va_control = -(stPI_Ia.Usk - stMControl_Var.f_BackCalGain*stMControl_Var.f_Speed_SatErr);
		}
		else //Compensate the back EMF
		{
			stMControl_Var.f_Va_control = -(stPI_Ia.Usk + stMControl_Var.f_BEMF);
			//stMControl_Var.f_Va_control = stPI_Ia.Usk - stMControl_Var.f_BackCalGain*stMControl_Var.f_Speed_SatErr + stMControl_Var.f_BEMF;
		}
		if (stMControl_Var.enum_mState != mSENSOR_CALIB)
		user_pwm_setvalue(&PWM_Var, stMControl_Var.f_Va_control,adc_Objt_var.f_Vbus_Filter, Unipolar);
	}
	/****************************************************************************/
	if(ui16_50msEvent == 1){
		ui16_50msEvent = 0;
			vRPM_Cal(&stQEI_Var, &stMachine_Var);
			N_Round=N_Round+(float)(stQEI_Var.f_Cnt_CAN/(52.0f*f_Gear_Ratio));
		  send_data_can(N_Round,ID_CAN);
			stQEI_Var.f_Cnt_CAN=0;
			stMControl_Var.f_MotorShaft_QEI_Speed = stQEI_Var.f_MotorShaft_rpm;
			stMControl_Var.f_GearShaft_QEI_Speed = stQEI_Var.f_GearShaft_rpm;
//		//Generate reference speed
//				fRampFcnGen(stMControl_Var.f_GearShaft_SpdSP, &stRamp_SpdSP);

//				//Run the PI Speed controller - Output of the speed controller is the reference amature current(Ampere)
//				fPI_Run(&stPI_Speed, stRamp_SpdSP.f_Output, stMControl_Var.f_GearShaft_QEI_Speed); 
//				if(stMControl_Var.enum_mState == mSTOP && stRamp_SpdSP.f_Output == 0.0f)
//				{
//						stMControl_Var.enum_mState = mSTANDSTILL;
//				}
				
	}
}
void Transmit_RS485(){	
    static uint32_t lastTransmitTick = 0;
    uint32_t currentTick = HAL_GetTick();

    // Check xem da truyen 200 ms chua
    if((currentTick - lastTransmitTick) >= 200){
        lastTransmitTick = currentTick;
        char buffer[100];
		  int len_gear = snprintf(buffer, sizeof(buffer),"Gear Shaft Speed : %.2f  , SP_speed= %.2f, Motor speed= %.2f \r\n", stMControl_Var.f_GearShaft_QEI_Speed,stMControl_Var.f_GearShaft_SpdSP,stQEI_Var.f_MotorShaft_rpm);
			HAL_UART_Transmit(&huart4, (uint8_t *)(buffer), len_gear, 100);
////					  int len_PID = snprintf(buffer, sizeof(buffer),"Ia= : %.2f ,i_speed= %.2f \r\n", stMControl_Var.f_Iak,stPI_Speed.Usk);
////			HAL_UART_Transmit(&huart4, (uint8_t *)(buffer), len_PID, 100);
								  int len_tourque = snprintf(buffer, sizeof(buffer),"Ia= : %.2f ,i_sp= %.2f \r\n", stMControl_Var.f_Iak,stRamp_IaSP.f_Output);
			HAL_UART_Transmit(&huart4, (uint8_t *)(buffer), len_tourque, 100);
//											  int len_tourque = snprintf(buffer, sizeof(buffer),"xung= : %.2d  \r\n", stQEI_Var.ui16t_Cnt_k);
//			HAL_UART_Transmit(&huart4, (uint8_t *)(buffer), len_tourque, 100);
		//	int len_motor = snprintf(buffer, sizeof(buffer), "Motor Shaft Speed : %.2f \r\n", stQEI_Var.f_MotorShaft_rpm);
		//	HAL_UART_Transmit(&huart4, (uint8_t*)(buffer), len_motor, 100);
//        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
    }
}



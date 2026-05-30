/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//Analog output
typedef struct 
{
	float f_Sensitivity;
	float f_Range1_Min, f_Range1_Max;				//Max range of the output voltage (V)
	float f_Range2_Min, f_Range2_Max;	
	float f_Offset1, f_Offset2;			
	float f_VO1, f_VO2;											//Output voltage (V)
	uint16_t ui_AO1, ui_AO2;								//Value to be sent to the DAC
}DAC_OUT;
/**********************************************************************************************/
//Quadrature Encoder
typedef struct
{
	uint16_t ui16t_ppr;								//Pulse per round of the motor shaft
	float Ts;													//Sampling time (sec)
	uint16_t ui16t_Cnt_k;							//Value of counter at time instance k and k_1
	uint16_t ui16t_Cnt_Ovf; 					//Overflow counter
	double f_Cnt_CAN; 
	float f_GearShaft_rpm;						//Gearbox shaft rpm
	float f_MotorShaft_rpm;						//Motor shaft rpm
	float f_GearRatio;
	int16_t i16t_Direction;						//Direction of the motor
	float f_PU_rpm, Const1;
	uint16_t ui16t_FirstCnt;
	uint16_t Auto_Reload;
}QEI;
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
	float f_SPk;																//Current and past value of setpoint
	float f_inputMax, f_inputMin;
	uint16_t ui16_T_Acc, ui16_T_Dec;						//Ramp up and down time (s)
	float f_delta_Acc, f_delta_Dec;							//step size in ACC and DEC mode
	uint16_t ui16_set_AccCnt, ui16_set_DecCnt;	//Set count - 10ms Resolution
	RAMP_STATE enum_State;
	float f_Output;
}RAMP;
/**********************************************************************************************/
//PI Controller
typedef struct
{
	float Kp, TI;										//Controller parameters	
	float Ks, D;										//Ks: Anti-windup Gain; D = 1-Ts/TI
	float Umax, Umin, Usk, Usk_1, Uk, Uk_1;	//Control signal
	float Ek, Ek_1, Esk, Esk_1;			//Error
	float Ts;												//Sampling time
	uint8_t ui8_MODE;								//Mode = 0: P, Mode = 1: PI
}cPI;
/**********************************************************************************************/
//Second-Order Unity-Gain Low Pass Filter
typedef struct
{
	float Ts;							//Sampling time
	float Fc; 						//Conrner frequency (Hz)
	float a1, a2, b1, b2;	//Parameters of the filter
	float alpha;
	float yk, yk_1, yk_2, uk, uk_1;
}LPF;
/**********************************************************************************************/
typedef struct
{
 float a0, a1, a2, b0, b1, b2;
 float f0, fs, omega;
 float r;
 float G; 
 float yk, yk_1, yk_2, uk, uk_1, uk_2;
}NotchFilter;
/**********************************************************************************************/
//Analog input - 12bit resolution
typedef struct 
{
	uint16_t AD1_Data[6];	
	
	float f_MaxVbus; 
	//KFactor for each ADC Channel - Normial value = 1.0
	float f_KFactor_Vbus, f_KFactor_Ia;	
	float f_KFactor_Speed_SP, f_KFactor_Torque_SP;
	//Vbus Divider Constant
	float f_Vbus_Divider;
	float f_Vbus_G;
	//Amature current Divider Constant
	float f_Ia_G;
	//Zero point of corresponding channel
	uint16_t ui16_AD_Ia_Offset, ui16_AD_Speed_SP_Offset, ui16_AD_Torque_SP_Offset;	
	uint16_t ui16_AD_VP_Offset, ui16_AD_VN_Offset;

	//Raw data
	float f_Vref_1_65_raw; 	//Reference voltage from Hall Effect current sensor
	float f_Vbus_Raw;											//Vol
	float f_Ia_Raw;												//Ampere
	int i_Ia_Raw;
	float f_Ia_Sensitivity;				//Sensitivity of the current sensor - V/A
	float f_PU_AN_SpdSP_raw, f_PU_AN_IaSP_raw, f_PU_AN_VaSP_raw; 	//Raw data of Speed, Torque and amature voltage setpoint - in Per unit
	//Filter data
	float f_Vref_1_65_Filter;	//SI Unit (V)
	float f_Ia_Filter, f_Vbus_Filter;	//SI unit
	float f_PU_AN_IaSP_Filter, f_PU_AN_SpdSP_Filter;	//Per unit
	float f_AD_Vref;			//Ref voltage of the ADC
}adc_Objt;
/**********************************************************************************************/
//Setpoint
typedef struct
{
	//Analog setpoint 
	float f_PU_AN_SpdSP, f_PU_AN_IaSP, f_PU_AN_VaSP;	//Per Unit (-1:1)range
	float f_SI_AN_MotorShaft_SpdSP, f_SI_AN_GearShaft_SpdSP, f_SI_AN_VaSP; //SI Unit
	float f_SI_AN_IaSP;																//SI Unit (Ampere)
	
	//Modbus setpoint
	float f_PU_Modbus_SpdSP, f_PU_Modbus_IaSP, f_PU_Modbus_VaSP;	//Per Unit (-1:1)range
	float f_SI_Modbus_MotorShaft_SpdSP, f_SI_Modbus_GearShaft_SpdSP, f_SI_Modbus_VaSP; //SI Unit
	float f_SI_Modbus_IaSP;																//SI Unit (Ampere)
	
	//CAN setpoint
	float f_PU_CAN_SpdSP, f_PU_CAN_IaSP, f_PU_CAN_VaSP;	//Per Unit (-1:1)range
	float f_SI_CAN_MotorShaft_SpdSP, f_SI_CAN_GearShaft_SpdSP, f_SI_CAN_VaSP; 	//SI Unit
	float f_SI_CAN_IaSP;																//SI Unit (Ampere)
	
	//PC-UART setpoint
	float f_PU_PC_UART_SpdSP, f_PU_PC_UART_IaSP, f_PU_PC_UART_VaSP;	//Per Unit (-1:1)range
	float f_SI_PC_UART_MotorShaft_SpdSP, f_SI_PC_UART_GearShaft_SpdSP, f_SI_PC_UART_VaSP; 									//SI Unit
	float f_SI_PC_UART_IaSP;																//SI Unit (Ampere)
	
}setpoint;
/**********************************************************************************************/
typedef enum  
{
	Low,
	High, 
	Rising, 
	Falling
}gpio_Event;
/**********************************************************************************************/
typedef enum
{
	mSENSOR_CALIB,
	mSTANDSTILL,
	mSTOP,  
	mRUN, 
	mFLT
}machine_State;
/**********************************************************************************************/
// system control
typedef struct{
  uint8_t ON;
	uint8_t OFF;
}mode;
/**********************************************************************************************/
//Manchine Control Mode
typedef enum
{
	OPEN_LOOP_VOL,
	TORQUE_NO_ENCODER, 
	TORQUE_ENCODER,
	SPEED_SENSORLESS,
	SPEED_ENCODER
}control_Mode;
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
	control_Mode enum_ctrMode;				//cMode = SPEED, Torque, Openloop_Vol
	machine_State enum_mState;				//State = SENSORCALIB, STOP, RUN, FAULT
	uint8_t ui8_sysENABLE, ui8_sysLOCAL_REMOTE;
	uint8_t ui8_BEMF_Compensate_EN;		//Enable back EMF compensation in Torque control mode
	
	/*Safety*/
	float f_If_H_Alarm, f_If_H_Fault;
	float f_If_L_Alarm, f_If_L_Fault;
	float f_Ia_H_Alarm, f_Ia_H_Fault;
	int i_Speed_H_Alarm, i_Speed_H_Fault;
	uint16_t ui16_Vdc_H_Alarm, ui16_Vdc_H_Fault , ui16_Vdc_L_Alarm, ui16_Vdc_L_Fault;
	
	/*Dynamic braking*/
	uint16_t ui16_DB_Hi, ui16_DB_Low;
	
	OP_State enum_If_State, enum_Ia_State, enum_Vdc_State, enum_Speed_State, enum_EXT_State;
	float f_Vdc_Nom;											//Nominal value of DC bus voltage
	uint16_t ui16_Vdc_Hi, ui16_Vdc_Lo;		//High and Low Level of the DC bus voltage, handle the charge state
	float f_GearShaft_SpdSP;							//RPM 
	float f_Va_SP;												//Vol
	float f_Ia_SP;
	int i_Dir;														//1: Forward, -1: Reverse
	int i_REVERSE_ON;											//1: REVERSE, 0: NORMAL

	float f_MotorShaft_EsSpeed;						//Estimation of motor shaft speed
	float f_GearShaft_EsSpeed;						//Estimation of motor shaft speed
	float f_MotorShaft_QEI_Speed;					//Motor shaft speed - measured by Encoder
	float f_GearShaft_QEI_Speed;					//Gear shaft speed - measured by Encoder
	float f_BEMF;													//Back EMF
	
	float f_EsSpeed_Kfactor;							//For sensorless speed correction
	float f_BackCalGain;
	float f_EsSpeed_Gain;
	float f_GearShaft_SpeedLimit;					//Used to limit speed in Torque Mode
	float f_Speed_SatErr;									//i_Speed_SatErr = i_EsSpeed - i_SpeedLimit
	float f_Va_control;										//Final control signal which is send to PWM module - Vol
	float f_Iak, f_Iak_1, f_dot_Ia, f_dot_Ia_Filter;
}machine_Control;
/**********************************************************************************************/
//Machine parameters
typedef struct
{
	float f_Ia_rated, f_Va_rated;
	float f_Overload_Factor, f_Ia_max, f_Motor_Torque_max;
	float f_Motor_Spd_rated, f_Motor_Spd_max;
	float f_OverSpeed_Factor;
	float f_Gear_Ratio;
	float f_GearShaft_Spd_rated, f_GearShaft_Spd_max, f_GearShaft_Torque_rated, f_GearShaft_Torque_Max;
	float f_La, f_Ra;
	float f_kPhiConst, f_Motor_Torque_rated;
	float f_Power_rated;	//Watt
}machine_Parameter;
/**********************************************************************************************/
//GPIO
typedef struct 
{
	GPIO_PinState dataK, dataK_1;	
	gpio_Event event;
}gpio_Objt;
/**********************************************************************************************/
//Timer 
typedef struct 
{
	uint16_t En;	//Enable the timer
	uint16_t SV;	//Set value
	uint16_t PV; 	//Present value
	uint16_t Output;
	uint16_t OvfCnt;
}timer_Objt;	
/**********************************************************************************************/
//PWM  
typedef struct
{
	uint8_t STATE;						//1-Enable; 0-Disable		
	int16_t D1, D2; 					//Duty cycle of Channel 1 & Channel 2
	float fD1, fD2;						//Normalized duty cycle
	float Unipolar_DMax;			//Manimum duty cycle in Unipolar mode
	float Bipolar_DMax;				//Manimum duty cycle in Bipolar mode
	uint16_t AutoReload;	
	uint16_t VLRated;					//Rated output dc voltage
}PWM;
/**********************************************************************************************/
typedef enum
{
	Unipolar,
	Bipolar
}PWM_Mode;
/**********************************************************************************************/
//usart data
typedef struct
{
	//Buffer
	uint8_t rxBuff[100];
	uint8_t txBuff[100];
	//Header
	uint8_t frameLength, groupID, senderAddr, receiverAddr, messType, dataLength;
	uint16_t messCount;
	//Data payload
	
	//State flow 
	uint8_t STATE;
	uint8_t RxFlag;
	uint8_t rxByte, rxPointer;
	uint16_t timeOut; 
}usartData;
/**********************************************************************************************/
//Nextion HMI
typedef struct
{
	/*Data send to Page 1*/
	int page1_z0_IaFbx100, page1_z1_IExcFbx100, page1_z2_SpeedFB, page1_z3_Vdc;
	int page1_x0_IaFbx100, page1_x2_IExcFbx100, page1_n0_SpeedEs, page1_n4_SpeedQEI, page1_n2_Vdc;
	int page1_x1_IaSPx100, page1_x3_IExcSPx100, page1_n1_VaSP, page1_n3_VfSP;
	int page1_x4_RPMx100, page1_x5_SpdSPx100;
	
	char page1_t14_ChargeState[20];
	char page1_t8_ControlMode[20];
	char page1_t10_MachineState[20];
	char page1_t6_ErrorMess[50];
	char page1_t7_LOCAL_REMOTE[50];
	
	/*Data send to Page 2*/
	uint16_t page2_n0_DI0, page2_n1_DI1, page2_n2_DI2, page2_n3_DI3;
	uint16_t page2_n4_DI4, page2_n5_DI5, page2_n6_DI6, page2_n16_FAULT;
	uint16_t page2_n7_DO0, page2_n8_DO1, page2_n9_DO2, page2_n10_DO3, page2_n11_DO4;
	uint16_t page2_n12_DB, page2_n13_EN, page2_n14_RLY, page2_n15_BYPASS;
	uint16_t page2_n17_SP2, page2_n18_RAMP2, page2_x0_SP1, page2_x1_RAMP1;
	uint16_t page2_n19_QEI_DIR, page2_n20_QEI_CNT, page2_n21_MessCnt;
	int page2_x2_LaxDia_dt;
	/*Data send to Page 3*/
	uint16_t page3_IaRefx10, page3_IaFbx10, page3_Ua, page3_IaRampx10;
	/*Data send to Page 4*/
	uint16_t page4_x0_RPM_SPx100, page4_x1_RPM_RAMPx100, page4_x2_RPM_FBx100, page4_x3_Iax100;
	
	uint32_t messCnt, rxVal;
	uint8_t UpdateEN;
	uint16_t ui16_GraphOffset;
}NEXTION;
/**********************************************************************************************/
// CAN
typedef struct{
double IaSP;
}CAN_data;
/**********************************************************************************************/
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
//NEXTION HMI interface
void vNEXTION_Init(NEXTION *pNEXVar);
void vNEXTION_SendVal(char *ID, int32_t Val, uint8_t dataType);
void vNEXTION_SendString(char *ID, char *string);
void vNEXTION_SendCmd(char *cmd, uint8_t val);
void vNEXTION_SendPCO(char *ID, int32_t val);
void vNEXTION_DisplayHandle(adc_Objt *pADC, machine_Control *pStateMachine,	RAMP *pRAMP_Ia, RAMP *pRAMP_SpdSP, QEI *pQEI, machine_Parameter *pMParas, NEXTION *pNEXVar);
/*Safety chain handle*/
void vSafetyChainHandle(adc_Objt *pAdcObjt, machine_Control *pMControl, QEI *pQEI);
void vStateMachine(void);
void vStateMachineConfig(machine_Control *pMControl, machine_Parameter *pMParas);
void vMachineParameterConfig(machine_Parameter *pMParas);

//Low pass filter 
void vLPF_Init(LPF *pLPF, uint16_t ConnerFrqHz, float samplingTime);	//Initialize LPF 
float fLPF_Run(LPF *pLPF, float input);																//Run LPF 

//GPIO functions
void vDIO_Init(void);			//Scan all input and setup output at startup
void vDI_Scan(void);			//Scan the proximity sensors in operation, every 10ms
gpio_Event eDI_EventDetect(GPIO_PinState k_Instance, GPIO_PinState k_1_Instance); //Check the even
void vDIO_Task_Handle(machine_Control *pMControl);
void vGPIO_Test(void);		//GPIO Test

//Timer interrupt handle
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);	

//PWM module
void user_pwm_Enable(PWM *pPWM_Var);
void user_pwm_Disable(PWM *pPWM_Var);
void user_pwm_init(PWM *PWM_Var, uint16_t Autoreload);	
void user_pwm_setvalue(PWM *PWM_Var, float SetVdc, float Vbus, uint8_t PWM_Mode); //Unipolar PWM for H-Bridge Converter
void User_MX_TIM1_Init(uint16_t PWM_Mode);

//Software Timer	
void vTim_Start(timer_Objt *ptim, uint16_t SV);				
void vTim_Stop(timer_Objt *ptim);											
void vTim_Scan(timer_Objt *pTIM);

//Usart handle
void vUsart4_RxHandle(usartData *pRxData);
void vUsart5_RxHandle(usartData *pRxData);

void vUsart_TxHandle(usartData *pTxData, uint8_t Port, uint8_t messType); //For Test Purpose

void vUsart_DataInit(usartData *pData);
void vUsart_Read(usartData *pRxData, uint8_t Port);								
void vUSART_RxTimeOutHandle(usartData *pRxData);
	

//Machine control
void vStateMachineLoop(void);																				
void setpointInit(setpoint *pSPVar, machine_Parameter *pMParas);
void vAmatureCurrentControl(void);
void vAmatureVolControl(void);
void Torque_control(void);
//PID controller
void vPI_Init(cPI *pPI, float TI, float Lambda, float Ts, float MaxSP, float Usat);	//PI Controller with Anti-Windup
float fPI_Run(cPI *pPI, float Ref, float Fb);

//ADC functions
void vAdc_Data_Handle(adc_Objt *pObjt, machine_Parameter *pMParas, setpoint *pSPVar);
void vAdc_DataInit(adc_Objt *pObjt, machine_Parameter *pMParas);			//Initialize adc data

//DAC functions
void vDAC_Init(DAC_OUT *pAO);																				//DAC data initialize
void vDAC_Write(DAC_OUT *pAO, uint8_t DAC_Channel, float Vout);			
void vDAC_CH1_IaWrite(DAC_OUT *pAO, uint8_t DAC_Channel, float Ia);											//Send current to analog output
//Config the Ramp Fcn genertor
void vRamp_Config(RAMP *pRamp, uint16_t Tacc_Sec, uint16_t Tdec_Sec, float inputMin, float inputMax, float Ts);
float fRampFcnGen(float SP, RAMP *pRamp);														//Ramp Fcn generator

//Incremental Encoder
void vQEI_Config(QEI *pQEI, int QEI_ppr, int QEI_mode, float f_GearRatio, float Ts);
void vRPM_Cal(QEI *pQEI, machine_Parameter *pMParas);
// commnunication
void send_data_can(double data, uint8_t ID);
void RS485_Init(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADDR1_Pin GPIO_PIN_13
#define ADDR1_GPIO_Port GPIOC
#define ADDR2_Pin GPIO_PIN_14
#define ADDR2_GPIO_Port GPIOC
#define ADDR3_Pin GPIO_PIN_15
#define ADDR3_GPIO_Port GPIOC
#define EXTI2_PULSE_FWD_Pin GPIO_PIN_2
#define EXTI2_PULSE_FWD_GPIO_Port GPIOC
#define EXTI2_PULSE_FWD_EXTI_IRQn EXTI2_IRQn
#define EXTI3_PULSE_REV_Pin GPIO_PIN_3
#define EXTI3_PULSE_REV_GPIO_Port GPIOC
#define EXTI3_PULSE_REV_EXTI_IRQn EXTI3_IRQn
#define SERVO_LOCK_Pin GPIO_PIN_0
#define SERVO_LOCK_GPIO_Port GPIOA
#define DI_1_Pin GPIO_PIN_1
#define DI_1_GPIO_Port GPIOA
#define DI_2_Pin GPIO_PIN_2
#define DI_2_GPIO_Port GPIOA
#define AD1_IN6_IA_SP_Pin GPIO_PIN_6
#define AD1_IN6_IA_SP_GPIO_Port GPIOA
#define AD1_IN7_VOL_SPD_SP_Pin GPIO_PIN_7
#define AD1_IN7_VOL_SPD_SP_GPIO_Port GPIOA
#define AD1_IN14_VP_Pin GPIO_PIN_4
#define AD1_IN14_VP_GPIO_Port GPIOC
#define AD1_IN15_VN_Pin GPIO_PIN_5
#define AD1_IN15_VN_GPIO_Port GPIOC
#define AD1_IN8_REF165_Pin GPIO_PIN_0
#define AD1_IN8_REF165_GPIO_Port GPIOB
#define AD1_IN9_ISENSE_Pin GPIO_PIN_1
#define AD1_IN9_ISENSE_GPIO_Port GPIOB
#define EXTI15_QEZ_Pin GPIO_PIN_15
#define EXTI15_QEZ_GPIO_Port GPIOB
#define EXTI15_QEZ_EXTI_IRQn EXTI15_10_IRQn
#define INH_2_Pin GPIO_PIN_8
#define INH_2_GPIO_Port GPIOC
#define INH_1_Pin GPIO_PIN_9
#define INH_1_GPIO_Port GPIOC
#define RS485_nRW_Pin GPIO_PIN_15
#define RS485_nRW_GPIO_Port GPIOA
#define RS485_TX4_Pin GPIO_PIN_10
#define RS485_TX4_GPIO_Port GPIOC
#define RS485_RX4_Pin GPIO_PIN_11
#define RS485_RX4_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_3
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_4
#define LED1_GPIO_Port GPIOB
#define ADDR0_Pin GPIO_PIN_9
#define ADDR0_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define MaxTIMER 20
#define NumOfDI 8

#define adcUsedChannel 6
#define adcMAX 4096

#define FrameStart 0x7A
#define FrameEnd 0x7F
#define RS485_Write 1
#define RS485_Read 0

#define USART_PENDING 2
#define USART_READING 1
#define USART_WAIT 0
#define PI 3.1416
#define NumOfLPF 20

#define DAC_DutyCycle 1
#define DAC_IfRefChannel 2
#define DAC_MAX_Vout 2.5

#define REMOTE_IO 0					//GPIO Port is used for control
#define LOCAL_HMI 1
#define REMOTE_MODBUS 2			//Control via MODBUS communication port
#define REMOTE_CAN 3				//Control via CAN communication port
#define REMOTE_WIFI	4				//Control via WIFI
#define REMOTE_LORA	5				//Control via LORA
#define REMOTE_PC_UART 6

#define MCU_ADDR 0x01				//Address of MCU in RS485 network

//NEXTION Return Data
#define NEX_RET_CMD_FINISHED            	0x01
#define NEX_RET_INVALID_CMD             	0x00
#define NEX_RET_INVALID_COMPONENT_ID    	0x02
#define NEX_RET_INVALID_PAGE_ID         	0x03
#define NEX_RET_INVALID_PICTURE_ID      	0x04
#define NEX_RET_INVALID_FONT_ID         	0x05
#define NEX_RET_INVALID_BAUD            	0x11
#define NEX_RET_INVALID_VARIABLE        	0x1A
#define NEX_RET_INVALID_OPERATION       	0x1B
#define NEX_RET_INVALID_ASSIGNMENT       	0x1C
#define NEX_RET_INVALID_EEPROM_OPERATION 	0x1D
#define NEX_RET_INVALID_PARA_QTY				 	0x1E
#define NEX_RET_TOO_LONG_VAR_NAME				 	0x23 
#define NEX_RET_TOO_BUFF_OVERFLOW				 	0x24 

#define RED 63488
#define YELLOW 65504
#define GREEN 2016
#define PURPLE 53594

#define GraphOffset 100
#define DATA_INT 0
#define DATA_XFLOAT 1

#define RS485_Port 4
#define PC_Port 5

#define FORWARD 1
#define REVERSE -1

#define P 0
#define P_I 1

#define MOTOR 0
#define LOAD 1
#define TEST_MODE 0
#define RUN_MODE 1

#define NumOfADC_Channel 6
#define AD_MAX_12Bit 4095u
#define AD_MAX_14Bit 16383u
#define AD_MAX_16Bit 65535u

#define VBUS_N 24.0f
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

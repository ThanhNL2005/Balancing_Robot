#include "main.h"

/*Timer object*/
extern timer_Objt Tim_10ms[MaxTIMER];				//10ms software timer
extern uint16_t tim10msTick;
extern uint8_t ID_CAN;
double test;
/*PWM*/
extern PWM PWM_Var;
extern TIM_HandleTypeDef htim1;

/*ADC and DAC data*/
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern adc_Objt adc_Objt_var;
extern volatile uint16_t ADC_CH1[6];
/*Setpoint variable*/
extern setpoint SP_Var;
//Quadrature Encoder
extern QEI stQEI_Var;	
//PI controller
extern cPI stPI_Speed, stPI_Ia;						
//State machine
extern machine_Control stMControl_Var;		
extern machine_Parameter stMachine_Var;
//RAMP Generator (Ampere with Ia, and rpm with Speed)
extern RAMP stRamp_Ia, stRamp_Speed, stRamp_If, stRamp_Vol;
/**********************************************************************************************/
void User_MX_TIM1_Init(uint16_t PWM_Mode)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim1.Init.Period = 39999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
	if(PWM_Mode == 1)
	{
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
	}
	else if(PWM_Mode == 2)
	{
		sConfigOC.OCMode = TIM_OCMODE_PWM2;
	}
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 10;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}
/**********************************************************************************************/
void vTim_Start(timer_Objt *ptim, uint16_t SV)			//Start timer
{
	if(ptim->En == 0)	//if the corresponding timer is in stop mode
	{
		ptim->En = 1;
		if(SV<1)
			SV = 1;
		ptim->SV = SV-1;
		ptim->Output = 0;
		ptim->PV = 0;
	}
}
/**********************************************************************************************/
void vTim_Stop(timer_Objt *ptim)									//Reset timer
{
	ptim->En = 0;
	ptim->SV = 0;
	ptim->Output = 0;
	ptim->PV = 0;
}
/**********************************************************************************************/
void vTim_Scan(timer_Objt *pTIM)	
{
	int i = 0;
	for(i=0;i<MaxTIMER;i++)									//Scan all declared timers
	{
		if(pTIM[i].En == 1)
		{
			if(++pTIM[i].PV > pTIM[i].SV)				//Overflow
			{
				pTIM[i].PV = 0;
				pTIM[i].Output = 1;								//Turn On the Timer Output
				if(++pTIM[i].OvfCnt > 65535)
				{
					pTIM[i].OvfCnt = 0;
				}
			}		
		}
	}
}
/**********************************************************************************************/
void user_pwm_init(PWM *pPWM_Var, uint16_t Autoreload)
{
	HAL_GPIO_WritePin(INH_1_GPIO_Port, INH_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(INH_2_GPIO_Port, INH_2_Pin, GPIO_PIN_RESET);
	pPWM_Var->AutoReload = Autoreload;	
	pPWM_Var->D1 = 0;
	pPWM_Var->D2 = 0;
	pPWM_Var->fD1 = 0.0f;
	pPWM_Var->fD2 = 0.0f;
	pPWM_Var->STATE = DISABLE;
	pPWM_Var->Unipolar_DMax = 0.95f*Autoreload;
	pPWM_Var->Bipolar_DMax = 0.95f*Autoreload;
}
/**********************************************************************************************/
void user_pwm_Enable(PWM *pPWM_Var)
{
	HAL_GPIO_WritePin(INH_1_GPIO_Port, INH_1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(INH_2_GPIO_Port, INH_2_Pin, GPIO_PIN_SET);
	pPWM_Var->STATE = ENABLE;
}
/**********************************************************************************************/
void user_pwm_Disable(PWM *pPWM_Var)
{
	HAL_GPIO_WritePin(INH_1_GPIO_Port, INH_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(INH_2_GPIO_Port, INH_2_Pin, GPIO_PIN_RESET);
	pPWM_Var->STATE = DISABLE;
}
/**********************************************************************************************/
void user_pwm_setvalue(PWM *pPWM_Var, float SetVdc, float Vbus, uint8_t PWM_Mode)
{
	switch(PWM_Mode) 
	{
		case Unipolar:
		{
			//Compute the duty cycle D1
			pPWM_Var->fD1 = SetVdc/Vbus; 			//fD1 range: -1 : 1
			pPWM_Var->fD2 = -pPWM_Var->fD1;
			pPWM_Var->D1 = pPWM_Var->fD1*pPWM_Var->AutoReload;
			pPWM_Var->D2 = pPWM_Var->fD2*pPWM_Var->AutoReload;
			//Duty cycle D1 and D2 restriction
			if(pPWM_Var->D1 > pPWM_Var->Unipolar_DMax)
				pPWM_Var->D1 = pPWM_Var->Unipolar_DMax;
			if(pPWM_Var->D2 > pPWM_Var->Unipolar_DMax)
				pPWM_Var->D2 = pPWM_Var->Unipolar_DMax;
			//Re-compute D1 & D2 
			if(pPWM_Var->D1 < 0)
				pPWM_Var->D1 = 0;
			else if(pPWM_Var->D2 < 0)
				pPWM_Var->D2 = 0;
			break;
		}
		case Bipolar:
		{
			//Compute the duty cycle D1
			pPWM_Var->fD1 = 0.5f*(SetVdc/Vbus + 1); // 0 <= fD1 <= 1
			pPWM_Var->D1 = pPWM_Var->fD1*pPWM_Var->AutoReload;
			//Duty cycle D1 verify
			if(pPWM_Var->D1 > pPWM_Var->Bipolar_DMax)
				pPWM_Var->D1 = pPWM_Var->Bipolar_DMax;
			else if(pPWM_Var->D1 < pPWM_Var->AutoReload-pPWM_Var->Unipolar_DMax)
				pPWM_Var->D1 = pPWM_Var->AutoReload-pPWM_Var->Unipolar_DMax;
			//Compute the duty cycle D2
			pPWM_Var->D2 = pPWM_Var->AutoReload - pPWM_Var->D1;	
			break;
		}			
	}
	TIM1->CCR1 = pPWM_Var->D2;
	TIM1->CCR2 = pPWM_Var->D1; 
}
/**********************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint16_t tim10ms_cnt = 0;
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);		
	/*USER CODE*/
		if(htim->Instance==TIM2){
//		send_data_can(stQEI_Var.ui16t_Cnt_CAN,ID_CAN);
//		stQEI_Var.ui16t_Cnt_CAN=0;
		}
	if(htim->Instance == TIM1) //PWM update interrupt - Ts = 50us, Fsw = 10kHz
	{
		/*For test only*/	
		//HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);	
		/*Machine control*/
//		vStateMachine();
		Torque_control();		
		//Scan the 10ms timer
		if(tim10msTick == 0)
		{
			if(++tim10ms_cnt == 99)
			{
//		send_data_can(test,ID_CAN);		
//		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
//				test++;
				tim10ms_cnt = 0;
				tim10msTick = 1;
			}
		}
	}
		
	if(htim->Instance==TIM3)				//Encoder pulse counter overflow
	{
		 stQEI_Var.ui16t_Cnt_Ovf = stQEI_Var.ui16t_Cnt_Ovf+1;
		if(stQEI_Var.ui16t_Cnt_Ovf > 65000)
		{
			stQEI_Var.ui16t_Cnt_Ovf = 0;
		}
	}
}

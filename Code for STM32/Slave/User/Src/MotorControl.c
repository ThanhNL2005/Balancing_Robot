
#include "math.h"
extern TIM_HandleTypeDef htim1;
extern int length2;
//extern float Ua[MAX_DATA];
/*Motor Parameter============================================================================*/
void DCMotor_Init(DCMotor *M){
	M->Ra=1.6; //Unit:Ohm
	M->La=0.000932; //Unit: H
	M->Ta=M->La/M->Ra;//time constant of Motor
	M->VaRated=24.0;  //Unit: V
	M->PRated=30;     //Unit: W
	M->TRated=0.0932*19.2; //Unit Nm
	M->wRated=12000*2*PI/(60*19.2);   // rad/s
	M->IaRated=3.6;
	M->KPhi=(M->VaRated-M->Ra*M->IaRated)/M->wRated;
}

/* Interrupt Flag============================================================================*/
void Interrupt_Init(Interrupt*f){
	f->current=0;
	f->speed=0;
}

/*Driver Enable and Stop=======================================================================*/
void Driver_Stop(void){
	HAL_GPIO_WritePin(INH_1_GPIO_Port,INH_1_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(INH_2_GPIO_Port,INH_2_Pin,GPIO_PIN_RESET);
}
void Driver_Enable(void){
	HAL_GPIO_WritePin(INH_1_GPIO_Port,INH_1_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(INH_2_GPIO_Port,INH_2_Pin,GPIO_PIN_SET);
}



/* Set up parameters for setpoint============================================================*/
void Setpoint_Init(Setpoint* sp){
//	sp->Isp=sp->Isp_filter=sp->Isp_raw=0;//0
	sp->Usp=0;
	sp->Ratio=0;
	sp->Duty=0;
	sp->Tsp=0;
}
/* Read ADC value============================================================================*/
void ADC_Init( AnalogToDigital *sensor){
  for(int i=0;i<4;i++){
		sensor->data[i]=0;
	}
	sensor->sensitivity=0.125;
	sensor->Vref_mcu=3.3;
	sensor->Vs_raw=0;
	sensor->Ia_raw=0;
	sensor->Vs_Voltage=0;
	sensor->Ia_Voltage=0;
	sensor->Ia=0;
	sensor->Vs=0;
}
void ReadCurrent( AnalogToDigital* sensor,LowPassFilter1*LPF1_Ia,LowPassFilter2*LPF2_Tsp,Setpoint* sp,int slave){ //void
  
//	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)sensor->data,3);
//	sensor->Voffset_raw=sensor->data[0];//ADC1_IN8
//	sensor->Voffset=(sensor->Voffset_raw/ADC_REGISTER)*sensor->Vref_mcu;
//	sensor->Ia_raw= sensor->data[1]-154;//ADC1_IN9
//	sensor->Ia_Voltage=(sensor->Ia_raw/ADC_REGISTER)*sensor->Vref_mcu;
//	sensor->Ia=(-sensor->Ia_Voltage+sensor->Voffset)/sensor->sensitivity;
//	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)sensor->data,3);

	sensor->Ia_raw= sensor->data[1];//ADC1_IN9
	sensor->Ia_Voltage=(sensor->Ia_raw/ADC_REGISTER)*sensor->Vref_mcu;
	
	sensor->Voffset_raw=sensor->data[0];//ADC1_IN8
	sensor->Voffset=(sensor->Voffset_raw/ADC_REGISTER)*sensor->Vref_mcu;
	
	
	//slave 1
//	float V=1.518;
//	if(sensor->Voffset == 0) V=0;
//	sensor->Ia=-(sensor->Ia_Voltage-2*sensor->Voffset+V)/sensor->sensitivity;
//	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
	
	
		//slave 2
//		float V=1.383;
//		if(sensor->Voffset == 0) V=0;
//	sensor->Ia=-(sensor->Ia_Voltage-2*sensor->Voffset+V)/sensor->sensitivity;
//	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
   
	 
	 	//slave 3
//	sensor->Ia=-(sensor->Ia_Voltage-2*sensor->Voffset+1.3797)/sensor->sensitivity;
//	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
  float V;
	switch(slave){
	  case 1:
  V=1.8;
	if(sensor->Voffset == 0) V=0;
	sensor->Ia=-(sensor->Ia_Voltage-2*sensor->Voffset+V)/sensor->sensitivity;
	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
		case 2:
  V=1.397;
	if(sensor->Voffset == 0) V=0;
	sensor->Ia=-(sensor->Ia_Voltage-2*sensor->Voffset+V)/sensor->sensitivity;
	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
		case 3:
	 V=1.397;
	if(sensor->Voffset == 0) V=0;
	sensor->Ia=-(sensor->Ia_Voltage-2*sensor->Voffset+V)/sensor->sensitivity;
	sensor->Ia_filter=LPF1_Run(LPF1_Ia,sensor->Ia);
	}
	
	
	
	sp->Tsp_raw=sensor->data[2];
	sp->Tsp=(sp->Tsp_raw/ADC_REGISTER)*sensor->Vref_mcu;
	
	sp->Tsp_filter=LPF2_Run(LPF2_Tsp,sp->Tsp);
	switch(slave){
		case 1:
	    //sp->Tsp_filter=(sp->Tsp_filter-2.15)*66/23;  //slave 1 and 2
	    if((0.24<sp->Tsp_filter)&&(sp->Tsp_filter<0.5))  sp->Tsp_filter-=0.25;
		  if((0.5<=sp->Tsp_filter)&&(sp->Tsp_filter<0.62))  sp->Tsp_filter-=0.2;
		  if((0.62<=sp->Tsp_filter)&&(sp->Tsp_filter<0.75))sp->Tsp_filter-=0.12;
		  if((0.75<=sp->Tsp_filter)&&(sp->Tsp_filter<0.9))sp->Tsp_filter-=0.05;
		  if(HAL_GPIO_ReadPin(DI_2_GPIO_Port,DI_2_Pin)==1) sp->Tsp_filter=0-sp->Tsp_filter;
		  break;
	  case 2:
			//sp->Tsp_filter=(sp->Tsp_filter-2.15)*66/23;
		  if((0.24<sp->Tsp_filter)&&(sp->Tsp_filter<0.5))  sp->Tsp_filter-=0.25;
		  if((0.5<=sp->Tsp_filter)&&(sp->Tsp_filter<0.62))  sp->Tsp_filter-=0.2;
		  if((0.62<=sp->Tsp_filter)&&(sp->Tsp_filter<0.75))sp->Tsp_filter-=0.12;
		  if((0.75<=sp->Tsp_filter)&&(sp->Tsp_filter<0.9))sp->Tsp_filter-=0.05;
		  if(HAL_GPIO_ReadPin(DI_2_GPIO_Port,DI_2_Pin)==1) sp->Tsp_filter=0-sp->Tsp_filter;
		  break;
		case 3:
	    //sp->Tsp_filter=(sp->Tsp_filter-1)*33/10; //slave 3
		  if((0.25<sp->Tsp_filter)&&(sp->Tsp_filter<0.5))  sp->Tsp_filter-=0.258;
		  if((0.5<=sp->Tsp_filter)&&(sp->Tsp_filter<0.75))  sp->Tsp_filter-=-0.13;
		  if((0.75<=sp->Tsp_filter)&&(sp->Tsp_filter<0.9))sp->Tsp_filter-=0.07;
		  if(HAL_GPIO_ReadPin(DI_2_GPIO_Port,DI_2_Pin)==1) sp->Tsp_filter=0-sp->Tsp_filter;
		  break;
	}
	//test
	if(sp->Tsp_filter>2.5) sp->Tsp_filter=2.5 ;   ///1
	sp->Tsp_filter=2.5; 
  sp->Isp_filter=sp->Tsp_filter/1;
}
/* PI controller for current loop with anti windup===========================================*/
void PIController_Init(PI_Current* cPI,float Ts,DCMotor M){
  cPI->damping=(float)(1/sqrt(2));
  cPI->Fsw=(float)FCLOCK/((htim1.Init.Period+1)*(htim1.Init.Prescaler+1));
  cPI->Td=1/cPI->Fsw;
	cPI->Tfi=1/(5*cPI->Fsw);
  cPI->Te=cPI->Td+cPI->Tfi;
  cPI->bandwidth=1/(2*cPI->damping*cPI->Te);
	cPI->ek =0; cPI->ek_1=0;cPI->esk=0;cPI->esk_1=0;
	cPI->uk =0; cPI->uk_1=0;cPI->usk=0;cPI->usk_1=0;
	cPI->Ts = Ts;
	cPI->uMax = V_SOURCE;
	cPI->uMin = -V_SOURCE;
	cPI->Kp =(float)M.Ra*M.Ta*cPI->bandwidth/(2*cPI->damping*10);
	cPI->Ti =M.Ta;
	cPI->Ka = 1/cPI->Kp;
	cPI->D =cPI->Ts/(2*cPI->Ti);
}

float PIController_Run(PI_Current* cPI,float ref,float fb,FeedForward* Ff,Encoder speed){
	cPI->ek=ref-fb;
	//error for anti-windup
	cPI->esk_1=cPI->ek_1+cPI->Ka*(cPI->usk_1-cPI->uk_1);
	cPI->esk=cPI->ek+cPI->Ka*(cPI->usk-cPI->uk);
	//phuong phap gian doan hoa bang pp hinh thang
	cPI->uk=cPI->usk_1+cPI->Kp*(cPI->ek-cPI->esk_1+cPI->D*(cPI->esk+cPI->esk_1));//+FeedForward_Run(Ff,speed.rad_s);
	//limit output
	if(cPI->uk > cPI->uMax)
		cPI->usk=cPI->uMax;
	else if(cPI->uk < cPI->uMin)
		cPI->usk=cPI->uMin;
	else
		cPI->usk=cPI->uk;
	//update data
	cPI->ek_1=cPI->ek;
	cPI->esk_1=cPI->esk;
	cPI->uk_1=cPI->uk;
	cPI->usk_1=cPI->usk;
	//return output of controller
	return cPI->usk;
}
/* Bipolar PWM method=======================================================================*/
void BipolarPWM(Setpoint* sp, AnalogToDigital*sensor,PI_Current* cPI,FeedForward* Ff,Encoder speed){
	sp->Usp=15;//PIController_Run(cPI,sp->Isp_filter,sensor->Ia_filter,Ff,speed);
//	if(length2<MAX_DATA){
//				Ua[length2]=sp->Usp;
//				length2++;
//	}
	sp->Ratio=sp->Usp/V_SOURCE;
	sp->Duty=(1.0+sp->Ratio)/2.0;     // fomular for bipolar method: Va=(1+2D)*Usp
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,(uint32_t)(htim1.Init.Period+1)*sp->Duty);
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2,(uint32_t)(htim1.Init.Period+1)*(1-sp->Duty));
}


/* Speed measurement===================================================================================*/
void Encoder_Init( Encoder* speed,uint32_t ppr,float gear_ratio,float SamplingTime){
	speed->ppr=ppr;
	speed->Ts=SamplingTime;
	speed->gear_ratio=gear_ratio;
	speed->overflow=0;
	speed->cnt=0;
	speed->cnt_1=0;
	speed->rad_s=0;
	speed->rpm=0;
	speed->delta_cnt=0;
	speed->direction=0;
	speed->f_encoder=0;
	speed->gain=60.0/(speed->gear_ratio*speed->ppr); // round per minute
}
void ReadSpeed(volatile Encoder*speed,TIM_HandleTypeDef*htim){ //void
	speed->cnt = __HAL_TIM_GetCounter(&htim3);
	speed->direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);
//	static uint8_t first_time=0;
//	if(first_time){
		if(speed->direction==0){   // Count up
			if(speed->overflow==0)
				speed->delta_cnt=speed->cnt-speed->cnt_1;
			else{
				speed->delta_cnt=speed->cnt-speed->cnt_1+speed->overflow*__HAL_TIM_GET_AUTORELOAD(htim);
			  speed->overflow=0;
			}
		}
		else{  //Count down
			if(speed->overflow==0)
				speed->delta_cnt=-speed->cnt+speed->cnt_1;
			else{
				speed->delta_cnt=speed->cnt_1-speed->cnt+speed->overflow*__HAL_TIM_GET_AUTORELOAD(htim);
			  speed->overflow=0;
			}
		}
//	}
//	else{
//		speed->delta_cnt=0;
//		speed->overflow=0;
//		first_time=1;
//	}
	speed->f_encoder=speed->delta_cnt/(8*speed->Ts);  // Problem
	speed->rpm=speed->gain*speed->f_encoder;
		
	if(speed->direction==0) speed->rad_s=speed->rpm*2*PI/60;
  else speed->rad_s= -speed->rpm*2*PI/60;
		
	speed->cnt_1=speed->cnt;
}
/* Filter for feedback current=========================================================================*/
void LPF1_Init(LowPassFilter1*LPF1,float TimeConstant, float Ts){
	LPF1->Ts=Ts;  
	LPF1->Fc=1/(2*PI*TimeConstant);        
	float alpha=2*PI*LPF1->Ts*LPF1->Fc;
	LPF1->a0=alpha/(alpha+2);
	LPF1->a1=alpha/(alpha+2);
	LPF1->b1=(alpha-2)/(alpha+2);
	LPF1->uk=LPF1->uk_1=0;
	LPF1->yk=LPF1->yk_1=0;
}
float LPF1_Run(LowPassFilter1*LPF1,float input){
	//Read LPF input
	LPF1->uk = input;	
	//Compute LPF output
	LPF1->yk = -LPF1->b1*LPF1->yk_1 + LPF1->a0*LPF1->uk + LPF1->a1*LPF1->uk_1; 
	//Update data
	LPF1->yk_1 = LPF1->yk; 
	LPF1->uk_1 = LPF1->uk;
	//Return LPF output
	return LPF1->yk;
}


/* FeedForward=========================================================================================*/
void FeedForward_Init(FeedForward* Ff,DCMotor *M,float Ts){
	Ff->KT=M->KPhi;
	Ff->Tfv=0.01;
	Ff->Ts=Ts;
	float alpha=Ff->Ts/Ff->Tfv;
	Ff->a0=alpha*Ff->KT/(alpha+2);
	Ff->a1=alpha*Ff->KT/(alpha+2);
	Ff->a2=(alpha-2)/(alpha+2);
	Ff->uk=Ff->uk_1=0;
	Ff->yk=Ff->yk_1=0;
}
float FeedForward_Run(FeedForward* Ff,float input){
	Ff->uk=input;
	Ff->yk= -Ff->a2*Ff->yk_1+Ff->a0*Ff->uk+Ff->a1*Ff->uk_1;
	Ff->yk_1=Ff->yk;
	Ff->uk_1=Ff->uk;
	return Ff->yk;
}




void LPF2_Init(LowPassFilter2*LPF2,float TimeConstant, float Ts){
	LPF2->Ts=Ts;  
	LPF2->Fc=1/(2*PI*TimeConstant);        
	float alpha=2*PI*LPF2->Ts*LPF2->Fc;
	float beta=1+alpha;
	LPF2->a0=alpha*alpha/(beta*beta);
	LPF2->b1=2/beta;
	LPF2->b2=-1/(beta*beta);
	LPF2->uk=0;
	LPF2->yk=LPF2->yk_1=LPF2->yk_2=0;
}
float LPF2_Run(LowPassFilter2*LPF2,float input){
	//Read LPF input
	LPF2->uk = input;	
	//Compute LPF output
	LPF2->yk = LPF2->b1*LPF2->yk_1+LPF2->b2*LPF2->yk_2+LPF2->a0*LPF2->uk; 
	//Update data
	LPF2->yk_2 = LPF2->yk_1; 
	LPF2->yk_1 = LPF2->yk; 
	//Return LPF output
	return LPF2->yk;
}










//void PI_FeedForward_Init(PI_FeedForward* cPIFf,DCMotor M){
//  cPIFf->damping=(float)(1/sqrt(2));
//  cPIFf->Fsw=(float)FCLOCK/((htim1.Init.Period+1)*(htim1.Init.Prescaler+1));
//  cPIFf->Td=1/cPIFf->Fsw;
//	cPIFf->Tfi=1/(5*cPIFf->Fsw);
//  cPIFf->Te=cPIFf->Td+cPIFf->Tfi;
//  cPIFf->wn=1/(2*cPIFf->damping*cPIFf->Te);
//	
//	cPIFf->Ts1 = Ts_Current;
//	cPIFf->Ts2 = Ts_Speed;
//	cPIFf->uMax = V_SOURCE;
//	cPIFf->uMin = -V_SOURCE;
//	cPIFf->Kp =(float)M.Ra*M.Ta*cPIFf->wn/(2*cPIFf->damping);
//	cPIFf->Ti =M.Ta;
//	cPIFf->Ki=cPIFf->Kp/cPIFf->Ti;
//	cPIFf->Ka = 5/cPIFf->Kp;
//	
//	cPIFf->KT=M.KPhi;
//	cPIFf->Tfv=0.01;
//	float alpha=cPIFf->Ts2/cPIFf->Tfv;
//	cPIFf->a0=alpha/(alpha+2);
//	cPIFf->a1=(alpha-2)/(alpha+2);
//	cPIFf->a2=cPIFf->Ki*cPIFf->Ts1;
//	
//	cPIFf->ek =0; cPIFf->ek_1 =0; cPIFf->ek_2 =0;
//	cPIFf->esk=0; cPIFf->esk_1=0; cPIFf->esk_2=0;
//	cPIFf->uk =0; cPIFf->uk_1 =0; cPIFf->uk_2 =0;
//	cPIFf->usk=0; cPIFf->usk_1=0; cPIFf->usk_2=0;
//	cPIFf->wk =0; cPIFf->wk_1 =0; cPIFf->wk_2 =0;
//}
//float PI_FeedForward_Run(PI_FeedForward* cPIFf,float ref,float fb,Encoder speed){
//	cPIFf->wk=speed.rad_s;
//	cPIFf->ek=ref-fb;
//	cPIFf->ek_2=cPIFf->esk_2;
//	cPIFf->ek_1=cPIFf->esk_1;
//	cPIFf->uk_2=cPIFf->usk_2;
//	cPIFf->uk_1=cPIFf->usk_1;
//		
//	//error for anti-windup
//	cPIFf->esk_2=cPIFf->ek_2-cPIFf->Ka*(cPIFf->uk_2-cPIFf->usk_2);
//	cPIFf->esk_1=cPIFf->ek_1-cPIFf->Ka*(cPIFf->uk_1-cPIFf->usk_1);
//	cPIFf->esk=cPIFf->ek-cPIFf->Ka*(cPIFf->uk-cPIFf->usk);
//	
//	float A=4*cPIFf->a0*cPIFf->uk_1 + cPIFf->a1*cPIFf->uk_2;
////	float B=cPIFf->a2*cPIFf->esk/2 + cPIFf->a2*cPIFf->a0*cPIFf->esk_1 + cPIFf->a2*cPIFf->a1*cPIFf->esk_2/2;
//	float C=cPIFf->Kp*cPIFf->ek + 4*cPIFf->Kp*cPIFf->a0*cPIFf->ek_1 + cPIFf->a1*cPIFf->ek_2;
//	float D=cPIFf->KT*cPIFf->a0*(cPIFf->wk-cPIFf->wk_2);
//	cPIFf->uk=A+C+D;
//	
//	//Saturation
//	if(cPIFf->uk > cPIFf->uMax)
//		cPIFf->usk=cPIFf->uMax;
//	else if(cPIFf->uk < cPIFf->uMin)
//		cPIFf->usk=cPIFf->uMin;
//	else
//		cPIFf->usk=cPIFf->uk;
//	
//	//update data
//	cPIFf->ek_2=cPIFf->ek_1;   cPIFf->ek_1=cPIFf->ek;
//	cPIFf->esk_2=cPIFf->esk_1; cPIFf->esk_1=cPIFf->esk;
//  cPIFf->uk_2=cPIFf->uk_1;   cPIFf->uk_1=cPIFf->uk;
//	cPIFf->usk_2=cPIFf->usk_1; cPIFf->usk_1=cPIFf->usk;
//	cPIFf->wk_2=cPIFf->wk_1;   cPIFf->wk_1=cPIFf->wk;
//	//return output of controller
//	return cPIFf->usk;
//}
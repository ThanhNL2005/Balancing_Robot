#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#include "main.h"
#define ADC_REGISTER 4095  // adc register has 12 bit
#define PI 3.14
#define FCLOCK 168000000.0 //frequency of clock 
#define V_SOURCE 24.0      // Voltage source that supplies for drive is 24V
#define Ts_Current 0.0001  //Sampling time for current loop         
#define Ts_Speed  0.01     // Sampling time for speed measurement
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1,htim3;
typedef struct{
	float Ra;
	float La;
	float Ta;
	float TRated;
	float KPhi;
	float VaRated;
	float IaRated;
	float PRated;
	float J;
	float wRated;
}DCMotor;
typedef struct{
	int current; // interrupt flag for current loop
	int speed;  // interrupt flag for speed measurement
}Interrupt;
typedef struct{
	float Isp,Isp_raw,Isp_filter,Usp;//Tsp; // setpoint for current,voltage
	float Ratio;       // ratio between output of PI controller and V_SOURCE
	float Duty;        //Duty cycle of PWM
	float Tsp,Tsp_raw,Tsp_filter
}Setpoint;

typedef struct{
	float Kp,Ti,D;
  float uk,uk_1,usk,usk_1;
	float ek,ek_1,esk,esk_1;
	float uMax,uMin;
	float Ts;  //Sampling time
	float Ka;  // Parameter for anti-windup
	float damping,bandwidth;
	float Fsw;    // frequency of driver
	float Td,Te;
	float Tfi; // Time constant of current sensors 
  float uff;
}PI_Current;
typedef struct{
	int16_t data[4];
	float sensitivity;
	float Voffset,Vref_mcu,Voffset_raw;
	float Vs,Ia;
	float Vs_raw,Ia_raw;
	float Vs_Voltage,Ia_Voltage;
	float Vs_filter,Ia_filter;
}AnalogToDigital;
typedef struct{
	uint32_t ppr;  // number of pulse per round
	float gear_ratio;
	float Ts;      // sampling time
  volatile uint32_t overflow ; // overflow counter 
  volatile int32_t direction;  // direction of rotation
	volatile float rad_s;        // speed motor rad/s
	float rpm;          //speed motor rpm
	float f_encoder;
  volatile uint32_t cnt,cnt_1,delta_cnt; // value of counter at time k and k-1, delta_cnt=cnt-cnt_1
	float gain;
}Encoder;
typedef struct{
	float Ts,Fc;  // Ts: sampling time, Fc: cutoff frequency
	float a0,a1,b1;
	float yk,yk_1,uk,uk_1; //yk=-b0*yk_1+a0*uk+a1*uk_1
}LowPassFilter1;// Low Pass Filter order 1

typedef struct{
	float Ts,Fc;  // Ts: sampling time, Fc: cutoff frequency
	float a0,b2,b1;
	float yk,yk_1,yk_2,uk; //yk=-b0*yk_1+a0*uk+a1*uk_1
}LowPassFilter2;// Low Pass Filter order 1


typedef struct{
	float uk,uk_1,yk,yk_1;
	float Tfv; 
	float KT,Ts;
	float a0,a1,a2;
}FeedForward;


/*Function*/
void DCMotor_Init(DCMotor *M);
void Interrupt_Init(Interrupt*f);
void Driver_Enable(void);
void Driver_Stop(void);
void Setpoint_Init(Setpoint* sp);
void ADC_Init( AnalogToDigital *sensor);
void ReadCurrent( AnalogToDigital* sensor,LowPassFilter1*LPF1_Ia,LowPassFilter2*LPF2_Tsp,Setpoint* sp,int slave); //void
void PIController_Init(PI_Current* cPI,float Ts,DCMotor M);
float PIController_Run(PI_Current* cPI,float ref,float fb,FeedForward* Ff,Encoder speed);
void BipolarPWM(Setpoint* sp, AnalogToDigital*sensor,PI_Current* cPI,FeedForward* Ff,Encoder speed);
void Encoder_Init( Encoder* speed,uint32_t ppr,float gear_ratio,float SamplingTime);
void FeedForward_Init(FeedForward* Ff,DCMotor *M,float Ts);
float FeedForward_Run(FeedForward* Ff,float input);
void ReadSpeed( volatile Encoder*speed,TIM_HandleTypeDef*htim);
void LPF1_Init(LowPassFilter1*LPF1,float TimeConstant, float Ts);
float LPF1_Run(LowPassFilter1*LPF,float input);
float LPF2_Run(LowPassFilter2*LPF2,float input);
void LPF2_Init(LowPassFilter2*LPF2,float TimeConstant, float Ts);


//float PI_FeedForward_Run(PI_FeedForward* cPIFf,float ref,float fb,Encoder speed);
//void PI_FeedForward_Init(PI_FeedForward* cPIFf,DCMotor M);
//typedef struct{
//	float Kp,Ti,Ki;
//  float uk,uk_1,uk_2,usk,usk_1,usk_2; // uk  is the output before saturaion,usk  is the output after saturaion
//	float ek,ek_1,ek_2,esk,esk_1,esk_2;
//	float uMax,uMin;
//	float Ts1,Ts2;  //Sampling time for current and speed
//	float Ka;  // Parameter for anti-windup
//	float damping,wn;
//	float Fsw;    // frequency of driver
//	float Td,Te;
//	float wk,wk_1,wk_2;
//	float Tfi,Tfv; // Time constant of current and speed sensors 
//	float KT;
//	float a0,a1,a2;
//}PI_FeedForward;
#endif
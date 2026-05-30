#include "main.h"


extern HWT906 HWT906_data;
extern HWT901 HWT901_data;
cPI pid_x;
cPI pid_theta;
cPI pid_psi;
cPD pd_x;
cPD pd_theta;
cPD pd_psi;
extern float angle_pitch;
extern float angle_psi;
extern float angle_psi_1;
float X_td=0.00f;
float r_w  = 0.075f;
extern SP_CAN can_sp;
extern GET_CAN can_get;
extern KA_Filter kf;
extern const float f_PID_Ts;			
double Torque_x,Torque_theta,Torque_psi,Torque;
const float pi=3.1415926;
float x[10000],psi[10000];
float pitch[10000];
float W1[10000];
float W2[10000];
int step;
uint8_t RS485_frame[12];







void control_theta_pid(){
	static uint16_t ui16_isrcnt_100ms = 0;
	static uint16_t ui16_100msEvent = 0;	
	static uint16_t ui16_isrcnt_200us = 0;
	static uint16_t ui16_200usEvent = 0;
		//Handle the isr counter
	if(++ui16_isrcnt_100ms > 49) //0.0025s - used for control loop
	{
		ui16_isrcnt_100ms = 0;
		ui16_100msEvent = 1;
	}
	if(++ui16_isrcnt_200us > 199) //0.005s - used for get data sensor
	{
		ui16_isrcnt_200us = 0;
		ui16_200usEvent = 1;
	}
	
	if (ui16_200usEvent==1){
		ui16_200usEvent=0;
		X_td=(float)((can_get.w1+can_get.w2)*(r_w/2));
		if (fabs(X_td) < 0.3){
		X_td=0.00f;
		}
//		PID_control(&pid_x,0.00f,X_td);	
		Torque_x=pid_x.Uk;		
	}
	if (ui16_100msEvent==1){
		ui16_100msEvent=0;
		// PID control position
		
		//PID control angle theta
    angle_pitch=HWT901_data.angle.x*pi/180.0f;
//		PID_control(&pid_theta,0.00f,angle_pitch);
		pid_theta.Ek=angle_pitch;
		pid_theta.Uk=500.0f*pid_theta.Ek + 5.0f*((pid_theta.Ek>0)-(pid_theta.Ek<0));
		Torque_theta=pid_theta.Uk;
		// PID control angle psi
		angle_psi=HWT901_data.angle.z*pi/180.0f;
		PID_control(&pid_psi,angle_psi_1,angle_psi);	
		Torque_psi=pid_psi.Uk;
		// limit in before 60 or pi/3
		if(angle_pitch >= pi/3|| angle_pitch <= -pi/3  ){Torque_theta=0;Torque_psi=0;}
		//convert to torque wheel
		can_sp.w1=(Torque_theta+ Torque_x +Torque_psi)/2.0f;
		can_sp.w2=-(Torque_theta + Torque_x -can_sp.w1);
//		can_sp.w1=Torque_theta/2.0f;
//		can_sp.w2=-Torque_theta/2.0f;	
//		can_sp.w1++;
//		can_sp.w2++;	
		
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2); // debug
//		pitch[step]=angle_pitch;
//		W1[step]=can_sp.w1;
//		W2[step]=can_sp.w2;
//		step++;
//		if(step>19999){

//			step=20000;
//		}
//		send_via_can(&can_sp);
		send_rs485(0x10,0x11,(float)can_sp.w1,(float)can_sp.w2,RS485_frame);
	}
}

void control__pid(){
	static uint16_t ui16_isrcnt_100ms = 0;
	static uint16_t ui16_100msEvent = 0;	
	static uint16_t ui16_isrcnt_200us = 0;
	static uint16_t ui16_200usEvent = 0;
	static uint16_t ui16_isrcnt_5ms = 0;
	static uint16_t ui16_5msEvent = 0;
		//Handle the isr counter
	if(++ui16_isrcnt_100ms > 99) //0.0025s - used for control loop
	{
		ui16_isrcnt_100ms = 0;
		ui16_100msEvent = 1;
	}
	if(++ui16_isrcnt_200us > 199) //0.005s - used for get data sensor
	{
		ui16_isrcnt_200us = 0;
		ui16_200usEvent = 1;
	}
		if(++ui16_isrcnt_100ms > 99) //0.0025s - used for control loop
	{
		ui16_isrcnt_5ms = 0;
		ui16_5msEvent = 1;
	}
	
		if (ui16_5msEvent==1){
		ui16_200usEvent=0;
//		HWT901_request();
		
	} 
		
	if (ui16_200usEvent==1){
		ui16_200usEvent=0;
//		HWT901_request();
		X_td=(float)((can_get.w1+can_get.w2)*(r_w/2));
		if (fabs(X_td) < 0.3){
		X_td=0.00f;
		}
//		PID_control(&pid_x,0.00f,X_td);	
		Torque_x=pid_x.Uk;		
	}
	if (ui16_100msEvent==1){
		ui16_100msEvent=0;
		// PID control position
		
		//PID control angle theta
    angle_pitch=HWT906_data.angle.x*pi/180.0f;
		PID_control(&pid_theta,0.00f,angle_pitch);
//		PID_run(&pd_theta,0.00f,angle_pitch);
		pid_theta.Ek=angle_pitch;
		pid_theta.Uk=50.0f*pid_theta.Ek + 1.0f*((pid_theta.Ek>0)-(pid_theta.Ek<0));
		Torque_theta=pid_theta.Uk;
		if(fabs(HWT901_data.angle.x)<0.2)Torque_theta=0;
		// PID control angle psi
		angle_psi=HWT901_data.angle.z*pi/180.0f;
		PID_control(&pid_psi,angle_psi_1,angle_psi);	
		Torque_psi=-pid_psi.Uk;
		if(fabs(pid_psi.Ek)<0.01)Torque_psi=0;
		// limit in before 60 or pi/3
		if(angle_pitch >= pi/3|| angle_pitch <= -pi/3  ){Torque_theta=0;Torque_psi=0;}
		//convert to torque wheel
		can_sp.w1=(Torque_theta+ Torque_x +Torque_psi)/2.0f; // Right wheel (0x11)
		can_sp.w2=-(Torque_theta + Torque_x -can_sp.w1);			// Left wheel (0x12)
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2); // debug
//		pitch[step]=angle_pitch;
//		W1[step]=can_sp.w1;
//		W2[step]=can_sp.w2;
//		step++;
//		if(step>19999){

//			step=20000;
//		}
//		send_via_can(&can_sp);
		send_rs485(0x10,0x11,(float)can_sp.w1,(float)can_sp.w2,RS485_frame);
	}
}



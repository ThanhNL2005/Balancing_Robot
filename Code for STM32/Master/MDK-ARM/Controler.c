#include "main.h"


extern HWT906 HWT906_data;
extern float PID_control (cPI *pPID,float setpoint, float measure,float umax,float umin);
extern pPID pid_theta;






void control_theta_pid(){
	static uint16_t ui16_isrcnt_50ms = 0;
	static uint16_t ui16_50msEvent = 0;	
	static uint16_t ui16_isrcnt_100us = 0;
	static uint16_t ui16_100usEvent = 0;
		//Handle the isr counter
	if(++ui16_isrcnt_50ms > 99) //50ms - used for gear shaft speed control loop
	{
		ui16_isrcnt_50ms = 0;
		ui16_50msEvent = 1;
	}
	if(++ui16_isrcnt_100us > 1) //250us - used for torque current control loop
	{
		ui16_isrcnt_100us = 0;
		ui16_100usEvent = 1;
	}
	
	if (ui16_100usEvent==1){
		ui16_100usEvent=0;
		
angle_pitch=HWT906_data.angle.x;
	}
	if (ui16_50msEvent==1){
		ui16_50msEvent=0;
		
		PID_control(pid_theta,0,angle_pitch,20,-20);
		pid_theta.Uk;
//		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_3);
	}
}
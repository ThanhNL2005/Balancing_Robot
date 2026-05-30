#include "main.h"
#include <stdio.h>
#include <math.h>

extern SP_CAN can_sp;
extern GET_CAN can_get;
extern float angle_pitch;
extern float angle_psi;
extern float angle_psi_1;
extern const float f_PID_Ts;
extern HWT906 HWT906_data;
extern const float pi;
float X_hsmc=0.00f;
float V_hsmc=0.00f;
float x_1=0.00f;
extern uint8_t RS485_frame[12];
// Parameter of two wheel mobile robot
const float mB = 14.575;    // mass of Pendulum Body
const float l  = 0.6;       // length of pendulum
const float r  = 0.0725;     // radius of wheels
const float mW = 0.512;     // mass of each wheel
const float d  = 0.45;      // distance about 2 wheel
const float g  = 9.81;      // gravity
const float I1 = (mB*d*d)/2;  // mass moment of inertia of the pendulum body
const float I2 = (mB*l*l)/2;  // mass moment of inertia of the pendulum body 
const float I3 = (mB*d*d)/2;  // mass moment of inertia of the pendulum body
const float J  = (mW*r*r)/2;  // mass moment of inertia of each wheel
const float K  = (mW*r*r)/4;  // mass moment of inertia of each wheel

// Hàm tính Fx = [f1, f2, f3] :
//
// f1 = { r^2*(mB*l^2 + I2)*[l*mB*sin(x3)*(x4^2+x6^2)] 
//      - l*mB*r^2*cos(x3)*[cos(x3)*sin(x3)*(mB*l^2+I1-I3)*x6^2 + g*l*mB*sin(x3)] }
//      -----------------------------------------------------------------------------
//                         denom_Fx
//
// f2 = { [ (cos(x3)*sin(x3)*(mB*l^2+I1-I3)*x6^2 + g*l*mB*sin(x3) ]*(2*J+mB*r^2+2*mW*r^2)
//      - l*mB*r^2*cos(x3)*[l*mB*sin(x3)*(x4^2+x6^2)] }
//      -----------------------------------------------------------------------------
//                         denom_Fx
//
// f3 = - { r^2*[l*mB*x2*x6*sin(x3) + 2*x4*x6*cos(x3)*sin(x3)*(mB*l^2+I1-I3)] }
//      -----------------------------------------------------------------------------
//                         denom2
//
// With:
//   denom_Fx = 2*I2*J + l^2*mB^2*r^2 + 2*J*l^2*mB + I2*mB*r^2 + 2*I2*mW*r^2 
//              - l^2*mB^2*r^2*cos(x3)^2 + 2*l^2*mB*mW*r^2
//
//   denom2   = J*d^2 + I3*r^2 + 2*K*r^2 + d^2*mW*r^2 
//              + I1*r^2*sin(x3)^2 - I3*r^2*sin(x3)^2 + l^2*mB*r^2*sin(x3)^2
//
void computeFx(float x2, float x3, float x4, float x6, float *f1, float *f2, float *f3) {
    float sin_x3 = sin(x3);
    float cos_x3 = cos(x3);
    float l2  = l * l;
    float mB2 = mB * mB;
    float r2  = r * r;
    
    float denom_Fx = (2 * I2 * J + l2 * mB2 * r2 + 2 * J * l2 * mB + I2 * mB * r2 + 2 * I2 * mW * r2 + 2 * l2 * mB * mW * r2) - (l2 * mB2 * r2 * cos_x3 * cos_x3);
    
    float term_common = l * mB;
    float numerator_f1 = r2 * (mB * l2 + I2) * term_common * sin_x3 * (x4*x4 + x6*x6)
                        - term_common * r2 * cos_x3 * (cos_x3 * sin_x3 * (mB * l2 + I1 - I3) * x6 * x6 + g * l * mB * sin_x3);
    *f1 = numerator_f1 / denom_Fx;
    
    float termA = (cos_x3 * sin_x3 * (mB * l2 + I1 - I3) * x6 * x6 + g * l * mB * sin_x3);
    float factor  = 2 * J + mB * r2 + 2 * mW * r2;
    float termB = term_common * r2 * cos_x3 * (term_common * sin_x3 * (x4*x4 + x6*x6));
    float numerator_f2 = termA * factor - termB;
    *f2 = numerator_f2 / denom_Fx;
    
    float d2 = d * d;
    float denom2 = J * d2 + I3 * r2 + 2 * K * r2 + d2 * mW * r2 + I1 * r2 * sin_x3 * sin_x3 - I3 * r2 * sin_x3 * sin_x3 + l2 * mB * r2 * sin_x3 * sin_x3;
    float numerator_f3 = r2 * (term_common * x2 * x6 * sin_x3 + 2 * x4 * x6 * cos_x3 * sin_x3 * (mB * l2 + I1 - I3));
    *f3 = - numerator_f3 / denom2;
}


// Hàm tính matrix Gx = inv(M)*Z :
//
// Gx du?c cho theo d?ng 3x3 v?i:
//
//  Gx = [  (r*(mB*l^2+I2))/denom_Fx,      (l*mB*r^2*cos(x3))/denom_Fx,       0;
//         -(l*mB*r*cos(x3))/denom_Fx,     -(2*J+mB*r^2+2*mW*r^2)/denom_Fx,    0;
//                  0,                              0,                    -(d*r)/(2*denom3) ]
//
// With:
//  denom_Fx nhu ? trên và
//  denom3 = J*d^2 + I3*r^2 + 2*K*r^2 + d^2*mW*r^2 + I1*r^2*sin(x3)^2 - I3*r^2*sin(x3)^2 + l^2*mB*r^2*sin(x3)^2
//
// Sau dó ta có:
//   g1 = Gx(1) + Gx(4)
//   g2 = Gx(2) + Gx(5)
//   g3 = Gx(9)
void computeG(float x3, float *g1, float *g2, float *g3) {
    float sin_x3 = sin(x3);
    float cos_x3 = cos(x3);
    float l2  = l * l;
    float mB2 = mB * mB;
    float r2  = r * r;
    
    float denom_Fx = (2 * I2 * J + l2 * mB2 * r2 + 2 * J * l2 * mB + I2 * mB * r2 + 2 * I2 * mW * r2 + 2 * l2 * mB * mW * r2) - (l2 * mB2 * r2 * cos_x3 * cos_x3);
    
    float Gx11 = (r * (mB * l2 + I2)) / denom_Fx;
    float Gx12 = (l * mB * r2 * cos_x3) / denom_Fx;
    float Gx21 = -(l * mB * r * cos_x3) / denom_Fx;
    float Gx22 = -(2 * J + mB * r2 + 2 * mW * r2) / denom_Fx;
    
    float d2 = d * d;
    float denom3 = J * d2 + I3 * r2 + 2 * K * r2 + d2 * mW * r2 + I1 * r2 * sin_x3 * sin_x3 - I3 * r2 * sin_x3 * sin_x3 + l2 * mB * r2 * sin_x3 * sin_x3;
    float Gx33 = -(d * r) / (2 * denom3);
    
    *g1 = Gx11 + Gx21;
    *g2 = Gx12 + Gx22;
    *g3 = Gx33;
}
void HSMC_control(void){
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
	if(++ui16_isrcnt_200us > 399) //0.02s - used for get data sensor
	{
		ui16_isrcnt_200us = 0;
		ui16_200usEvent = 1;
	}
	
	if (ui16_200usEvent==1){
		ui16_200usEvent=0;

	}
		if (ui16_100msEvent==1){
		ui16_100msEvent=0;
    float x1 = (float)((can_get.w1+can_get.w2)*(r/2)) ;
    float	x2 = (x1-x_1)*400;
    float x3 = HWT906_data.angle.x * pi / 180.0f;
    float x4 = HWT906_data.angular_velocity.x * pi / 180.0f;
    float x5 = HWT906_data.angle.z * pi / 180.0f - angle_psi_1;
    float x6 = HWT906_data.angular_velocity.z * pi / 180.0f;
    
    
    float f1, f2, f3, g1, g2, g3;
    computeFx(x2, x3, x4, x6, &f1, &f2, &f3);
    computeG(x3, &g1, &g2, &g3);
	  // control parameter HSMC 
    float c1 = 10; // sliding sufface of x
    float c2 = 15; // sliding sufface of theta
    float k2 = 12; // switching control 
    float lambda1 = 5; 
    float beta1 = 5;
		float C3=1;   // sliding sufface of psi
		float ETA2=10; // sign of HSMC
		float ETA3=2; // sign of SMC psi
		float K3=1;
    //
		float x1d = 0.0f,x1d_d = 0.0f;
		float x3d = 0.0f ,x3d_d = 0.0f;
		float x5d =angle_psi_1 ,x5d_d = 0.0f;
		float x1d_dd,x3d_dd,x5d_dd;
    // error parameter
    float e1 = x1 - x1d;
    float e2 = x2 - x1d_d;
    float e3 = x3 - x3d;
    float e4 = x4 - x3d_d;
    float e5 = x5 - x5d;
    float e6 = x6 - x5d_d;
    
    float s1 = c1 * e1 + e2;
    float s2 = c2 * e3 + e4;
    float s3 = C3 * e5 + e6;
    
    float s = lambda1 * s1 + beta1 * s2;
    
    // out control
    float ueq1 = (-c1 * e2 - f1 + x1d_dd) / g1;
    float ueq2 = (-c2 * e4 - f2 + x3d_dd) / g2;
    float usw  = (-k2 * s - ETA2 * ((s > 0) - (s < 0)) - lambda1 * g1 * ueq2 - beta1 * g2 * ueq1) / (lambda1 * g1 + beta1 * g2);
    
    float u1 = ueq1 + ueq2 + usw;
    float u2 = (1 / g3) * (-f3 + x5d_dd - ETA3 * ((s3 > 0) - (s3 < 0)) - K3 * s3 - C3 * e6);
		can_sp.w1=(u1+u2)/2.0f;
		can_sp.w2=-(u1-u2)/2.0f;	
		send_rs485(0x10,0x11,(float)can_sp.w1,(float)can_sp.w2,RS485_frame);
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
			}
		}


#include "main.h"
#include <stdio.h>
#include <math.h>
#include <string.h> 

extern SP_CAN can_sp;
extern GET_CAN can_get;
extern RAMP fRamp_x;
extern float angle_pitch;
extern float angle_psi_1;
extern HWT906 HWT906_data;
extern const float pi;
float x_1 = 0.00f;
extern uint8_t RS485_frame[12];

// Khởi tạo thông số vật lý
void TwoWBMR_Init(TWBMR *robot)
{
	robot->mB = 10.05f; //21.0f
	robot->l = 0.7f;
	robot->r = 0.0625f;
	robot->mW = 0.35f;
	robot->d = 0.35f;
	robot->g = 9.81f;

	robot->I1 = (robot->mB * robot->d * robot->d) / 2.0f;
	robot->I2 = (robot->mB * robot->l * robot->l) / 2.0f;
	robot->I3 = (robot->mB * robot->d * robot->d) / 2.0f;

	robot->J = (robot->mW * robot->r * robot->r) / 2.0f;
	robot->K = (robot->mW * robot->r * robot->r) / 4.0f;
}

void compute_f_smc(const TWBMR *params, const HSMC *state, F *f)
{
	// Retrieve required state values
	float x2 = state->x2;
	float x3 = state->x3;
	float x4 = state->x4;
	float x6 = state->x6;

	// Compute sine and cosine of x3
	float sin_x3 = lookup_sin(x3);
	float cos_x3 = lookup_cos(x3);

	// Compute squared values for r and l, and d^2
	float r2 = params->r * params->r;
	float l2 = params->l * params->l;
	float d2 = params->d * params->d;

	// Compute Term1:
	// Term1 = r^2 * (mB * l^2 + I2) * (l * mB * sin(x3) * (x4^2 + x6^2))
	float term1 = r2 * (params->mB * l2 + params->I2)
			* (params->l * params->mB * sin_x3 * (x4 * x4 + x6 * x6));

	// Compute Term2:
	// Term2 = l * mB * r^2 * cos(x3) * [ cos(x3)*sin(x3)*(mB * l^2 + I1 - I3)*x6^2 + g*l*mB*sin(x3) ]
	float term2 = params->l * params->mB * r2 * cos_x3
			* (cos_x3 * sin_x3 * (params->mB * l2 + params->I1 - params->I3)
					* (x6 * x6) + params->g * params->l * params->mB * sin_x3);

	// Compute the denominator (#1):
	// #1 = 2*I2*J + l*mB*r^2 + 2*J*l*mB + I2*mB*r^2 + 2*I2*mW*r^2 - l*mB*r^2*cos(x3) + 2*l*mB*mW*r^2
	float D = 2.0f * params->I2 * params->J + params->l * params->mB * r2
			+ 2.0f * params->J * params->l * params->mB
			+ params->I2 * params->mB * r2 + 2.0f * params->I2 * params->mW * r2
			- params->l * params->mB * r2 * cos_x3
			+ 2.0f * params->l * params->mB * params->mW * r2;

	// Calculate f1 as (Term1 - Term2) divided by D
	f->f1 = (term1 - term2) / D;
	// Compute the first part of the numerator:
	// Part1 = ( cos(x3)*sin(x3)*(mB*l^2 + I1 - I3)*x6^2 + g*l*mB*sin(x3) )
	float part1 = cos_x3 * sin_x3 * (params->mB * l2 + params->I1 - params->I3)
			* (x6 * x6) + params->g * params->l * params->mB * sin_x3;

	// Compute the multiplier factor for part1:
	// Factor = (2J + mB*r^2 + 2mW*r^2)
	float factor = 2.0f * params->J + params->mB * r2 + 2.0f * params->mW * r2;

	// Compute the second part of the numerator:
	// Part2 = l*mB*r^2*cos(x3) * ( l*mB*sin(x3)*(x4^2 + x6^2) )
	float part2 = params->l * params->mB * r2 * cos_x3
			* (params->l * params->mB * sin_x3 * (x4 * x4 + x6 * x6));

	// Calculate f2 as (Part1*factor - Part2) divided by D
	f->f2 = (part1 * factor - part2) / D;
	//
	// term_common = l*mB (as used in the f1 computation)
	float term_common = params->l * params->mB;

	// Compute the numerator:
	// Numerator = r^2 * [ l*mB*x2*x6*sin(x3) + 2*x4*x6*cos(x3)*sin(x3)*(mB*l^2 + I1 - I3) ]
	float numerator = r2
			* (term_common * x2 * x6 * sin_x3
					+ 2.0f * x4 * x6 * cos_x3 * sin_x3
							* (params->mB * l2 + params->I1 - params->I3));

	// Compute the denominator:
	// Denom = J*d^2 + I3*r^2 + 2*K*r^2 + d^2*mW*r^2 + I1*r^2*sin(x3)^2 - I3*r^2*sin(x3)^2 + l*mB*r^2*sin(x3)
	float denom = params->J * d2 + params->I3 * r2 + 2.0f * params->K * r2
			+ d2 * params->mW * r2 + params->I1 * r2 * (sin_x3 * sin_x3)
			- params->I3 * r2 * (sin_x3 * sin_x3)
			+ params->l * params->mB * r2 * sin_x3;

	// Calculate f3 as the negative ratio of numerator to denominator
	f->f3 = -numerator / denom;
}

void compute_G_smc(const TWBMR *params, const HSMC *state, G *g)
{
	// Extract common parameters and state variables
	float x3 = state->x3;
	// Precompute trigonometric values
	float cos_x3 = lookup_cos(x3);
	float cos_x3_sq = cos_x3 * cos_x3;
	float sin_x3 = lookup_sin(x3);
	float sin_x3_sq = sin_x3 * sin_x3;
	float l2 = params->l * params->l;
	float r2 = params->r * params->r;
	float MB2 = params->mB * params->mB;

	// Compute denominator for g1 and g2
	float denominator_g1g2 = 2.0f * params->I2 * params->J + l2 * MB2 * r2
			+ 2.0f * params->J * l2 * params->mB + params->I2 * params->mB * r2
			+ 2.0f * params->I2 * params->mW * r2 - l2 * MB2 * r2 * cos_x3_sq
			+ 2.0f * l2 * params->mB * params->mW * r2;

	// Calculate g1
	float numerator_g1 = (r2 * (params->mB * l2 + params->I2))
			+ (params->l * params->mB * r2 * cos_x3);
	g->g1 = numerator_g1 / denominator_g1g2;

	// Calculate g2
	float termA = 2.0f * params->J + params->mB * r2 + 2.0f * params->mW * r2;
	float termB = params->l * params->mB * params->r * cos_x3;
	g->g2 = -(termA + termB) / denominator_g1g2;

	// Compute denominator for g3
	float denominator_g3_part = params->J * params->d * params->d
			+ params->I3 * r2 + 2.0f * params->K * r2
			+ params->d * params->d * params->mW * r2
			+ params->I1 * r2 * sin_x3_sq - params->I3 * r2 * sin_x3_sq
			+ l2 * params->mB * r2 * sin_x3_sq;
	float denominator_g3 = 2.0f * denominator_g3_part;

	// Calculate g3
	g->g3 = -(params->d * params->r) / denominator_g3;
}

void SMC_control(TWBMR *param, SMC *smc, F *f, G *g)
{
	float sampling_period = 0.0025f;
	smc->x1 = (float) ((can_get.w1 + can_get.w2) * (param->r / 2));
	smc->x2 = (x_1 - smc->x1) / sampling_period; // Dùng x_1 cũ
	x_1 = smc->x1; // Cập nhật x_1
	smc->x3 = HWT906_data.angle.x * pi / 180.0f;
//			if(fabs(HWT906_data.angle.x)<0.3)hsmc->x3=0;
	smc->x4 = HWT906_data.angular_velocity.x * pi / 180.0f;
//			if(fabs(HWT906_data.angle.x)<0.3)hsmc->x4=0;
	smc->x5 = HWT906_data.angle.z * pi / 180.0f;
	smc->x6 = HWT906_data.angular_velocity.z * pi / 180.0f;
//        if(fabs(HWT901_data.angle.x) < 0.1){smc->x3=0;smc->x4=0;}
	compute_f_smc(param, smc, f);
	compute_G_smc(param, smc, g);
	float x1d = 0.0f, x1d_d = 0.0f, x1d_dd = 0.0f;
	float x3d = 0.0f, x3d_d = 0.0f, x3d_dd = 0.0f;
	float x5d = angle_psi_1, x5d_d = 0.0f, x5d_dd = 0.0f;

	smc->e1 = -(smc->x1 - x1d);
	smc->e2 = -(smc->x2 - x1d_d);
	smc->e3 = smc->x3 - x3d;
	smc->e4 = smc->x4 - x3d_d;
	smc->e5 = smc->x5 - x5d;
	smc->e6 = smc->x6 - x5d_d;

	smc->s1 = smc->c1 * smc->e1 + smc->e2;
	smc->s2 = smc->c2 * smc->e3 + smc->e4;
	smc->s3 = smc->c3 * smc->e5 + smc->e6;

	smc->u_x = (1 / g->g1)
			* (-smc->c1 * smc->e2 + x1d_dd
					- smc->ETA1 * ((smc->s1 > 0) - (smc->s1 < 0))
					- smc->k1 * smc->s1);
	smc->u_theta = (1 / g->g2)
			* (-smc->c2 * smc->e4 + x3d_dd
					- smc->ETA2 * ((smc->s2 > 0) - (smc->s2 < 0))
					- smc->k2 * smc->s2);
	smc->u_psi = (1 / g->g3)
			* (-smc->c3 * smc->e6 + x5d_dd
					- smc->ETA3 * ((smc->s3 > 0) - (smc->s3 < 0))
					- smc->k3 * smc->s3);

	smc->u1 = smc->u_x + smc->u_theta;
	smc->u2 = smc->u_psi;

	can_sp.w1 = (smc->u1 + smc->u2) / 2.0f;
	can_sp.w2 = -(smc->u1 - smc->u2) / 2.0f;
	send_rs485(0x10, 0x11, can_sp.w1, can_sp.w2, RS485_frame);
    Toggle_Led(1);
}

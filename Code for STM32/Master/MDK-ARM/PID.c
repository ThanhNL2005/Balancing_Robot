#include "main.h"
/**********************************************************************************************/
extern float Tf_PID;
void vPI_Init(cPI *pPI, float TI, float Lambda, float Ts, float MaxSP, float Usat)
{
	//Note: 0 < Lambda < 1
	pPI->Uk_1 = 0; pPI->Uk = 0; pPI->Usk_1 = 0;
	pPI->Ek = 0; pPI->Ek_1 = 0; 
	pPI->Esk = 0; pPI->Esk_1 = 0; 
	
	pPI->Umax = Usat; pPI->Umin = -Usat;
	//Sampling time (sec)
	pPI->Ts = Ts;	
	//pPI->Kp = KP; 
	pPI->Kp = Lambda*pPI->Umax/MaxSP;
	pPI->Ks = pPI->Kp;
	//Integral time (sec)
	pPI->TI = TI; 
	if(TI <=0.0f)
	{
		pPI->ui8_MODE = P;		//Proportional only
		pPI->TI = 0.0f;
		pPI->D = 0.0f;
	}
	else
	{
		pPI->ui8_MODE = P_I;	//Proportional-Integral									
		pPI->D = 1.0f - pPI->Ts/pPI->TI;
	}
}
/**********************************************************************************************/
float fPI_Run(cPI *pPI, float Ref, float Fb)
{
	pPI->Ek = Ref - Fb;
	//pPI->Ek = -Ref + Fb;
	switch(pPI->ui8_MODE)
	{
		case P: //Proportional controller
		{
			pPI->Uk = pPI->Kp*pPI->Ek;
			if(pPI->Uk > pPI->Umax)
					pPI->Usk = pPI->Umax;
			else if(pPI->Uk < pPI->Umin)
					pPI->Usk = pPI->Umin;
			else 
					pPI->Usk = pPI->Uk;
			break;
		}
		case P_I:
		{
			pPI->Esk_1 = pPI->Ek_1 + (1/pPI->Ks)*(pPI->Usk_1-pPI->Uk_1);
			pPI->Uk = pPI->Usk_1 +pPI->Kp*(pPI->Ek-pPI->D*pPI->Esk_1);

			if(pPI->Uk > pPI->Umax)
					pPI->Usk = pPI->Umax;
			else if(pPI->Uk < pPI->Umin)
					pPI->Usk = pPI->Umin;
			else 
					pPI->Usk = pPI->Uk;
			//update data
			pPI->Ek_1 = pPI->Ek;
			pPI->Uk_1 = pPI->Uk;
			pPI->Usk_1 = pPI->Usk;
			break;
		}
	}
	return pPI->Usk;
}
void set_PID(cPI *pPID,float kp, float ki, float kd, float n,    float ts,   float umax, float umin) 
{
    pPID->Kp = kp;     // P
    pPID->Ki = ki;     // I
    pPID->Kd = kd;     // D
    
    pPID->Ks = n;      // N

    pPID->Ts = ts;


    pPID->Umax = umax;
    pPID->Umin = umin;


    pPID->Ek   = 0.0f;
    pPID->Ek_1 = 0.0f;
    pPID->Esk  = 0.0f; // iTerm
    pPID->Usk  = 0.0f; // dTerm
    pPID->Uk   = 0.0f;
    pPID->Uk_1 = 0.0f;
}

float PID_control(cPI *pPID, float setpoint, float measure)
{
     pPID->Ek = -setpoint + measure;
    // 2.1 Ph?n P
    float pTerm = pPID->Kp * pPID->Ek;

    //     yI(k) = yI(k-1) + I*Ts*e(k)
//    float iTerm = pPID->Esk + pPID->Ki * pPID->Ts * pPID->Ek;
	float iTerm = pPID->Esk + (pPID->Ki * pPID->Ts/2) * (pPID->Ek+pPID->Ek_1);
//	  float iTerm = pPID->Ki *(pPID->Esk +  (pPID->Ts/2) * (e_k+pPID->Ek_1*2+pPID->Ek_2));

    //     yD(k) = (1 - N*Ts)*yD(k-1) + D*N*( e(k) - e(k-1) )
//    float dTerm = (1.0f - pPID->Ks * pPID->Ts) * pPID->Usk + (pPID->Kd * pPID->Ks) * ( pPID->Ek - pPID->Ek_1 );
//    float dTerm = pPID->Kd*pPID->Ts*(pPID->Ek-pPID->Ek_1);
		float dTerm = (2*pPID->Kd/pPID->Ts)*(pPID->Ek-pPID->Ek_1);
    float u = pTerm + iTerm + dTerm;

//    if(fabs(iTerm)>fabs(pTerm*3)) {iTerm=pTerm*3;}	
    if (u > pPID->Umax) {u = pPID->Umax;}
    if (u < pPID->Umin) {u = pPID->Umin;}

    pPID->Ek_1 = pPID->Ek;   
    pPID->Ek_2=pPID->Ek_1;    
    pPID->Esk  = iTerm;     
    pPID->Usk  = dTerm;     
    pPID->Uk_1 = pPID->Uk;  
    pPID->Uk   = u;

    return u;
}
 
//void PID_init(cPD *pPID,uint8_t mode, float Ts,float K_p,float K_i,float K_d, float N,float max, float min)
//{
//  pPID->Ts=Ts;
//	pPID->Kp=K_p;
//	pPID->Ki=K_i;
//	pPID->Kd=K_d;
//	pPID->N=N;
//	pPID->Umax=max;
//	pPID->Umin=min;
//	pPID->T=1/(pPID->N*pPID->Ts-1);
//	pPID->ui8_MODE = mode;
//}
//float PID_run(cPD *pPID, float setpoin, float measure)
//{
//	pPID->Ek=setpoin-measure;
//		switch(pPID->ui8_MODE)
//	{
//			case P:
//			{
//			}
//			case P_I:
//			{
//			}
//			case P_I_D:
//			{
//				pPID->Uk=(pPID->Kp/pPID->T)*(pPID->Ek*(1+pPID->Ts)-pPID->Ek_1) + pPID->Kd*pPID->N*(pPID->Ek-pPID->Ek_1)/pPID->T + pPID->Uk_1/pPID->T;
//			}
//	}
//	    if (pPID->Uk > pPID->Umax) {pPID->Uk = pPID->Umax;}
//    if (pPID->Uk < pPID->Umin) {pPID->Uk = pPID->Umin;}
//		
//		    pPID->Ek_1 = pPID->Ek;           
//				pPID->Uk_1 = pPID->Uk;
//		return pPID->Uk;
//}


void PID_init(cPD *pPID, uint8_t mode,
              float Ts, float K_p, float K_i, float K_d,
              float N, float max, float min)
{

    pPID->Ts = Ts;
    pPID->Kp = K_p;
    pPID->Ki = K_i;
    pPID->Kd = K_d;
    pPID->N  = N;
    pPID->Umax = max;
    pPID->Umin = min;
    pPID->ui8_MODE = mode;

}


float PID_run(cPD *pPID, float setpoint, float measure)
{

    pPID->Ek = -setpoint + measure;


    if (pPID->ui8_MODE == 0) {

        pPID->Uk = pPID->Kp * pPID->Ek;
    }
    else if (pPID->ui8_MODE == 1) {
			
			pPID->UP=pPID->Kp * pPID->Ek;
			pPID->UI=pPID->UI_1+pPID->Ki * pPID->Ek*pPID->Ts;
      pPID->Uk = pPID->UP+pPID->UI;
			
    }
    else {

			pPID->UP=pPID->Kp * pPID->Ek;
			pPID->UI=pPID->UI_1+pPID->Ki * pPID->Ek*pPID->Ts;
			pPID->UD=(pPID->UD_1+pPID->Kd*pPID->N*(pPID->Ek-pPID->Ek_1))/(1+pPID->Ts*pPID->N);
      pPID->Uk = pPID->UP+pPID->UI+pPID->UD ;
    }
    if (pPID->Uk > pPID->Umax) pPID->Uk = pPID->Umax;
    if (pPID->Uk < pPID->Umin) pPID->Uk = pPID->Umin;

    pPID->Ek_2 = pPID->Ek_1;
    pPID->Ek_1 = pPID->Ek;
		pPID->UI_1=pPID->UI;
		pPID->UD_1=pPID->UD_1;
    pPID->Uk_1 = pPID->Uk;

    return pPID->Uk;
}



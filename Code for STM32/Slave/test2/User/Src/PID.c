#include "main.h"
/**********************************************************************************************/
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


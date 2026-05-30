#include "main.h"

#define COMBINE_2BYTES(high, low) ((int16_t)(((int16_t)(high) << 8) | (low)))

extern ESP32 esp32;
extern int start;
extern uint8_t RS485_frame[12];
uint8_t CalculateChecksum(uint8_t *data, uint8_t length)
{
    uint16_t sum = 0;
	    for(uint8_t i = 0; i < length - 2; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF); 
}


void CreateAndSendFrame(UART_HandleTypeDef *huart, uint8_t id,HWT906 *hwt906_data ,HWT901 *hwt901_data,SP_CAN *can,GET_CAN *get_can,uint8_t *frame, TWBMR *robot)
{


    uint8_t startByte = 0xAA;  
    uint8_t endByte   = 0x55;  

    // 1. Start byte
    frame[0] = startByte;

    // 2. ID
    frame[1] = id;

    // 3. Length 
    frame[2] = 0x08; 
    // data
    int data=(int)(-(hwt906_data->angle.x)*100); 
	  frame[4]=(uint8_t)((data>>8)&0xFF);
	  frame[3]=(uint8_t)(data&0xFF);
	  data=(int)(hwt901_data->angle.z*100);
	  frame[6]=(uint8_t)((data>>8)&0xFF);
	  frame[5]=(uint8_t)(data&0xFF);
		data=(int)(hwt906_data->angular_velocity.x*100);
	  frame[8]=(uint8_t)((data>>8)&0xFF);
	  frame[7]=(uint8_t)(data&0xFF);
		data=(int)(hwt901_data->angular_velocity.z*100);
	  frame[10]=(uint8_t)((data>>8)&0xFF);
	  frame[9]=(uint8_t)(data&0xFF);
		data=(int)(-(get_can->w1+get_can->w2)*6.28*(0.073/2.0)*50);
	  frame[12]=(uint8_t)((data>>8)&0xFF);
	  frame[11]=(uint8_t)(data&0xFF);
		data=(int)(can->w1*100*(-1.0f));
	  frame[14]=(uint8_t)((data>>8)&0xFF);
	  frame[13]=(uint8_t)(data&0xFF);
		data=(int)(can->w2*100);
	  frame[16]=(uint8_t)((data>>8)&0xFF);
	  frame[15]=(uint8_t)(data&0xFF);
    // 5. Checksum 
    frame[17] = Modbus_CRC16(frame,19);

    // 6. End byte
    frame[18] = endByte;

    // Trasmit frame via UART
//    HAL_UART_Transmit_DMA(huart, frame,19);
		HAL_UART_Transmit(huart, frame, 19, 1);
}

void ESP_to_STM(uint8_t *frame,cPI *x,cPI *theta,cPI *psi,ESP32 *esp){
	if(frame[0] == 0xBB){
    uint8_t data_type = frame[1];
    uint8_t frame_type = frame[2];
		uint8_t count_type = frame[3];
	if(frame_type == 0x00){
	switch (data_type)
	{
		case run_stop:
		{
			if(frame[3] == 0x01)start=1;
			if(frame[3] == 0x00){
				start=0;
				send_rs485(0x10, 0x11, 0, 0,RS485_frame);
			}
      break;			
		}
		case PID_x:
		{
			if(count_type == 0x01)x->Kp=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			if(count_type == 0x02)x->Ki=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			if(count_type == 0x03)x->Kd=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
		  break;
		}
		case PID_theta:
		{
			if(count_type == 0x01)theta->Kp=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			if(count_type == 0x02)theta->Ki=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			if(count_type == 0x03)theta->Kd=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
		  break;
		}
		case PID_psi:
		{
			if(count_type == 0x01)psi->Kp=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			if(count_type == 0x02)psi->Ki=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			if(count_type == 0x03)psi->Kd=(float)(COMBINE_2BYTES(frame[5],frame[4]/100));
			break;
		}
		case hsmc_e:
		{
			
		}		
		case hsmc_d:
		{
			
		}		
		case hsmc_c:
		{
			
		}
	}
	esp->rec_mode=0;
}
		if(frame_type == 0x01){
		switch (data_type)
	{
		case PID_x:
		{
			x->Kp=(float)(COMBINE_2BYTES(frame[4],frame[3]/100));
			x->Ki=(float)(COMBINE_2BYTES(frame[6],frame[5]/100));
			x->Kd=(float)(COMBINE_2BYTES(frame[8],frame[7]/100));
			break;
		}
		case PID_theta:
		{
			theta->Kp=(float)(COMBINE_2BYTES(frame[4],frame[3]/100));
			theta->Ki=(float)(COMBINE_2BYTES(frame[6],frame[5]/100));
			theta->Kd=(float)(COMBINE_2BYTES(frame[8],frame[7]/100));
			break;
		}
		case PID_psi:
		{
			psi->Kp=(float)(COMBINE_2BYTES(frame[4],frame[3]/100));
			psi->Ki=(float)(COMBINE_2BYTES(frame[6],frame[5]/100));
			psi->Kd=(float)(COMBINE_2BYTES(frame[8],frame[7]/100));
			break;
		}
		case hsmc_e:
		{
			
		}		
		case hsmc_d:
		{
			
		}		
		case hsmc_c:
		{
			
		}	
	}
	esp->rec_mode=0;
}
}
	}

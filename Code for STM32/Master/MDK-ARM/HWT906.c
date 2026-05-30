#include "main.h"
#include <stdint.h>
#include <string.h>


// byte config type data HWT with 200hz
#define TYPE_ACCELERATION 0x51
#define TYPE_ANGULAR_VELOCITY 0x52
#define TYPE_ANGLE 0x53
#define RX_SIZE 33   /* config legth data_frame*/ /* correct with frequence 200hz*/
#define COMBINE_2BYTES(high, low) ((int16_t)(((int16_t)(high) << 8) | (low)))
// config variable 
extern UART_HandleTypeDef huart7;
extern HWT906 HWT906_data;
extern uint8_t RxData[RX_SIZE];  // store data process
extern uint8_t rxBuffer[RX_SIZE] ; // store ram data
extern DMA_HandleTypeDef hdma_uart7_rx;
float angle_pitch ;
float angle_psi;
// void
extern float convert_acceleration(int16_t raw);
extern float convert_angular_velocity(int16_t raw);
extern float convert_angle(int16_t raw);
extern void process_frame(HWT906 *hwt906_data, uint8_t *data);
extern void read_frame(uint8_t *buffer, uint8_t size);

//extern void angle_referenc();



void read_frame(uint8_t *buffer, uint8_t size) {
    if (size <= 1) return; 

    uint8_t temp = buffer[0]; 


    for (int i = 0; i < size - 1; i++) {
        buffer[i] = buffer[i + 1];
    }


    buffer[size - 1] = temp;
}
// arrange data buffer from HWT906
void rearrange_frame(uint8_t *dmaBuffer, uint8_t *orderedBuffer)
{
    int startIndex = -1;
    for (int i = 0; i < RX_SIZE; i++) { // find start byte
        if (dmaBuffer[i] == 0x55 && dmaBuffer[(i + 1) % RX_SIZE] == 0x51) {
            startIndex = i;
            break;
        }
    }   
    if (startIndex == -1) {
        return; 
    }
    
    // rearrange data
    for (int i = 0; i < RX_SIZE; i++) {
        orderedBuffer[i] = dmaBuffer[(startIndex + i) % RX_SIZE];
    }
}
void angle_referenc(void)
{
	HAL_DMA_Abort(&hdma_uart7_rx);
    uint8_t cmd_unlock[] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
    uint8_t cmd_calib[]  = {0xFF, 0xAA, 0x01, 0x08, 0x00};
    uint8_t cmd_save[]   = {0xFF, 0xAA, 0x00, 0x00, 0x00};

    // tranmit via uart
    HAL_UART_Transmit(&huart7, cmd_unlock, sizeof(cmd_unlock), 100);      
    HAL_UART_Transmit(&huart7, cmd_calib, sizeof(cmd_calib), 100);
    HAL_UART_Transmit(&huart7, cmd_save, sizeof(cmd_save), 100);
		memset(rxBuffer, 0, sizeof(rxBuffer));
		HAL_UART_Receive_DMA(&huart7, rxBuffer, RX_SIZE);
		HAL_Delay(100);
}

void restart_HWT906(void)
{
    uint8_t cmd_unlock[] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
    uint8_t cmd_restart[]  = {0xFF, 0xAA, 0x00, 0xFF, 0x00};
    uint8_t cmd_save[]   = {0xFF, 0xAA, 0x00, 0x00, 0x00};

    // tranmit via uart
    HAL_UART_Transmit(&huart7, cmd_unlock, sizeof(cmd_unlock), 100);      
    HAL_UART_Transmit(&huart7, cmd_restart, sizeof(cmd_restart), 100);
    HAL_UART_Transmit(&huart7, cmd_save, sizeof(cmd_save), 100);
		memset(rxBuffer, 0, sizeof(rxBuffer));
		HAL_UART_Receive_DMA(&huart7, rxBuffer, RX_SIZE);
//		HAL_Delay(100);
}

// read data from HWT906 (acceleration,angular_velocity,angle)
void process_frame(HWT906 *hwt906_data, uint8_t *data) {
    // provess each frame (13 byte)
    for (int i = 0; i < RX_SIZE/11; i++) {
        uint8_t *frame = &RxData[i * 11]; 
        uint8_t sum = 0;
        for (int j = 0; j < 10; j++) {
            sum += frame[j];
        }
        // check start byte and byte sum
        if (frame[0] != 0x55 || sum != frame[10]) {

//				restart_HWT906();
			
        }
        else {
        uint8_t type = frame[1];
        int16_t data1 = COMBINE_2BYTES(frame[3], frame[2]);
        int16_t data2 = COMBINE_2BYTES(frame[5], frame[4]);
        int16_t data3 = COMBINE_2BYTES(frame[7], frame[6]);

        // caculator data 
        switch (type) {
            case TYPE_ACCELERATION:
                hwt906_data->acceleration.x = ((float)data1 / 32768.0f) * 16.0f;
                hwt906_data->acceleration.y = ((float)data2 / 32768.0f) * 16.0f;
                hwt906_data->acceleration.z = ((float)data3 / 32768.0f) * 16.0f;
                break;

            case TYPE_ANGULAR_VELOCITY:
                hwt906_data->angular_velocity.x = ((float)data1 / 32768.0f) * 2000.0f;
                hwt906_data->angular_velocity.y = ((float)data2 / 32768.0f) * 2000.0f;
                hwt906_data->angular_velocity.z = ((float)data3 / 32768.0f) * 2000.0f;
                break;

            case TYPE_ANGLE:
                hwt906_data->angle.x = ((float)data1 / 32768.0f) * 180.0f;
                hwt906_data->angle.y = ((float)data2 / 32768.0f) * 180.0f;
                hwt906_data->angle.z = ((float)data3 / 32768.0f) * 180.0f;
                break;

            default:
                break;
        }
			}
    }
}



	


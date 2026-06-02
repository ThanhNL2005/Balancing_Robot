#pragma once
#include "main.h"

typedef struct {
  uint8_t buff[32];
  uint8_t rlength; // Số phần tử nhận được thực tế trong buff (đã trừ các phần
                   // tử sót lại của vòng trước)
} VS_struct;
/******************************************************************************/
#define short_frame_len 9
#define long_frame_len 12
#define VS_Start_Byte 0xBB
#define VS_End_Byte 0x66
#define RING_BUFF_LEN 64
/******************************************************************************/
static inline uint8_t ring_increase(volatile uint8_t val) {
  return (val + 1) & (RING_BUFF_LEN - 1);
}
void Send_Data_To_ESP32(UART_HandleTypeDef *huart, HWT906 *hwt906_data,
                        SP_CAN *can, GET_CAN *get_can, uint8_t *frame);
void Read_And_Process_VSData(void);
void Process_VS_Data(void);
/******************************************************************************/
extern volatile uint8_t VS_head;
extern volatile uint8_t VS_tail;
extern VS_struct VS_Buffer[RING_BUFF_LEN];
extern volatile uint8_t tx_esp32;
extern uint8_t tx_frame[20];
extern uint8_t rx_frame[32];

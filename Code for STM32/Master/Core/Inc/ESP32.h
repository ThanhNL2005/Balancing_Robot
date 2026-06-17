#pragma once
#include "main.h"

typedef struct {
  uint8_t buff[32];
  uint8_t rlength; // Số phần tử thực sự vừa nhận được
} VSRxData_t;

/*************************************************************************************************************************/
enum ESP32_CONFIG {
  SHORT_FRAME_LEN = 9,
  LONG_FRAME_LEN = 12,
  DOWNLINK_START_BYTE = 0xBB,
  END_BYTE = 0xED,
  RING_BUFF_LEN = 64
};

/*************************************************************************************************************************/
static inline uint8_t RingIncrease(volatile uint8_t val) { return (val + 1) & (RING_BUFF_LEN - 1); }
void SendDataToESP32(UART_HandleTypeDef *p_huart, HWT906 *p_hwt906_data, SP_CAN *p_can, GET_CAN *p_get_can, uint8_t *p_frame);
void ReadAndProcessVSData(void);

/*************************************************************************************************************************/
extern volatile uint8_t vs_head;
extern volatile uint8_t vs_tail;
extern VSRxData_t vs_buffer[RING_BUFF_LEN];
extern volatile uint8_t g_tx_esp32_flag;
extern uint8_t tx_frame[20];
extern uint8_t rx_frame[32];

#include "ESP32.h"
#include "main.h"
#include <string.h>

/*************************************************************************************************************************/
// HWT906
#define RX_SIZE 33
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart8;
extern DMA_HandleTypeDef hdma_uart8_tx;
extern DMA_HandleTypeDef hdma_uart8_rx;
extern HWT906 HWT906_data;
extern uint8_t RxData[RX_SIZE];   // store data process
extern uint8_t rxBuffer[RX_SIZE]; // store ram data
extern DMA_HandleTypeDef hdma_uart7_rx;

/*************************************************************************************************************************/
// Hàm ngắt khi nhận đủ kích thước dữ liệu
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  // Xử lý dữ liệu HWT906
  if (huart->Instance == UART7) {
    memcpy(RxData, rxBuffer, RX_SIZE);
    rearrange_frame(rxBuffer, RxData);
    HAL_UART_Receive_DMA(&huart7, rxBuffer, sizeof(rxBuffer)); // Kích hoạt lại cổng nhận dữ liệu
    process_frame(&HWT906_data, RxData);
  }
}
/*************************************************************************************************************************/
// Hàm ngắt khi dừng nhận dữ liệu
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
  // Xử lý nhận dữ liệu từ ESP32
  if (huart->Instance == USART6) {
    if (size >= SHORT_FRAME_LEN && RingIncrease(vs_head) != vs_tail) {
      memcpy(vs_buffer[vs_head].buff, rx_frame, size); // Lưu vào buffer
      vs_buffer[vs_head].rlength = size;
      vs_head = RingIncrease(vs_head);
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_frame, sizeof(rx_frame)); // Kích hoạt lại cổng nhận dữ liệu
  }
}

/*************************************************************************************************************************/
// Hàm ngắt khi gặp lỗi UART
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART6) {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_frame, sizeof(rx_frame));
  }
}
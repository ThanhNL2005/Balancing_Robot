#include "ESP32.h"
#include "main.h"
#include <string.h>


/******************************************************************************/
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

/******************************************************************************/
// Hàm ngắt khi nhận đủ kích thước dữ liệu
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  // Xử lý dữ liệu HWT906
  if (huart->Instance == UART7) {
    memcpy(RxData, rxBuffer, RX_SIZE);
    rearrange_frame(rxBuffer, RxData);
    HAL_UART_Receive_DMA(&huart7, rxBuffer,
                         RX_SIZE); // Kích hoạt lại cổng nhận dữ liệu
    process_frame(&HWT906_data, RxData);
  }
}
/******************************************************************************/
// Hàm ngắt khi dừng nhận dữ liệu
void HAL_UARTEx_RxEventCallback(
    UART_HandleTypeDef *huart,
    uint16_t size) // size: Số byte nhận được tính từ lúc kích hoạt lại
{
  // Xử lý nhận dữ liệu từ ESP32
  if (huart->Instance == USART6) {
    if (size >= short_frame_len && ring_increase(VS_head) != VS_tail) {
      memcpy(VS_Buffer[VS_head].buff, rx_frame, size); // Lưu vào buffer
      VS_Buffer[VS_head].rlength = size;
      VS_head = ring_increase(VS_head);
    }
    HAL_UARTEx_ReceiveToIdle_DMA(
        &huart6, rx_frame, sizeof(rx_frame)); // Kích hoạt lại cổng nhận dữ liệu
  }
}

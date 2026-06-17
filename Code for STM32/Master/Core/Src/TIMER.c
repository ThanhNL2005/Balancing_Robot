#include "ESP32.h"
#include "main.h"

extern uint8_t RS485_frame[50];
volatile uint16_t tim50msTick;

/*************************************************************************************************************************/
// Hàm phục vụ ngắt Timer
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  UNUSED(htim);

  if (htim->Instance == TIM5) // Ts = 2.5ms
  {
    // 1. Điều khiển động cơ
    if (control) {
      SMC_control(&parameter_TWBMR, &control_smc, &f_funtion, &g_funtion);
    } else {
      // Gửi lệnh dừng động cơ
      extern SP_CAN can_sp;
      can_sp.w1 = 0.0f;
      can_sp.w2 = 0.0f;
      send_rs485(0x10, 0x11, can_sp.w1, can_sp.w2, RS485_frame);
    }

    // 2. Gửi dữ liệu lên ESP32 (T = 50 ms)
    if (++tim50msTick == 20) {
      g_tx_esp32_flag = 1;
      tim50msTick = 0;
    }
  }
}

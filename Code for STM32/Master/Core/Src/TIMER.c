#include "main.h"
#include "ESP32.h"

// ESP32
volatile uint16_t tim50msTick;

/**********************************************************************************************/
// Hàm phục vụ ngắt Timer
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	UNUSED(htim);

	if (htim->Instance == TIM5) // Ts = 50us
	{
		// Tính toán và gửi lệnh điều khiển
		if (control)
		{
			static uint16_t isr_cnt_2_5ms = 0;
			static uint16_t event_2_5ms = 0;

			if (++isr_cnt_2_5ms == 50) // Bộ chia tần (T = 2.5 ms, f = 400 Hz)
			{
				isr_cnt_2_5ms = 0;
				event_2_5ms = 1;
			}

			if (event_2_5ms == 1)
			{
				event_2_5ms = 0;
				SMC_control(&parameter_TWBMR, &control_smc, &f_funtion, &g_funtion);
			}
		}

		// Bộ đếm thời gian gửi dữ liệu lên ESP32 (T = 50 ms)
		if (++tim50msTick == 1000)
		{
			tx_esp32 = 1;
			tim50msTick = 0;
		}
	}
}

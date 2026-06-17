#include "ESP32.h"
#include <stdint.h>
#include <string.h>

/*************************************************************************************************************************/
volatile uint8_t g_tx_esp32_flag; // Cờ báo cần gửi dửi liệu lên ESP32
uint8_t tx_frame[20];             // Truyền dữ liệu ra cổng UART6
uint8_t rx_frame[32];             // Nhận dữ liệu từ cổng UART6
uint8_t g_vs_flag;                // Cờ báo đọc thành công 1 frame từ VS
uint8_t vs_frame[32];             // Pop dữ liệu từ VS Buffer
volatile uint8_t vs_head = 0;
volatile uint8_t vs_tail = 0;
VSRxData_t vs_buffer[RING_BUFF_LEN]; // Ring Buffer nhận dữ liệu VS từ cổng UART6

/*************************************************************************************************************************/
// Hàm tính mã Modbus CRC 16
uint16_t ModbusCRC16(uint8_t *p_frame, uint8_t start_i, uint8_t end_i) {
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = start_i; pos <= end_i; pos++) {
    crc ^= (uint16_t)p_frame[pos];
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

/*************************************************************************************************************************/
// Hàm tạo và gửi dữ liệu lên ESP32
void SendDataToESP32(UART_HandleTypeDef *p_huart, HWT906 *p_hwt906_data, SP_CAN *p_can, GET_CAN *p_get_can,
                     uint8_t *p_frame) {

  uint8_t up_start_byte = 0xAA;
  uint8_t end_byte = 0xED;

  // 1. Start byte
  p_frame[0] = up_start_byte;

  // 2. Data
  int data = (int)(-(p_hwt906_data->angle.x) * 100.0f);
  p_frame[2] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[1] = (uint8_t)(data & 0xFF);
  data = (int)(p_hwt906_data->angle.z * 100.0f);
  p_frame[4] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[3] = (uint8_t)(data & 0xFF);
  data = (int)(p_hwt906_data->angular_velocity.x * 100.0f);
  p_frame[6] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[5] = (uint8_t)(data & 0xFF);
  data = (int)(p_hwt906_data->angular_velocity.z * 100.0f);
  p_frame[8] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[7] = (uint8_t)(data & 0xFF);
  data = (int)(-(p_get_can->w1 + p_get_can->w2) * 6.28f * (0.073f / 2.0f) * 50.0f);
  p_frame[10] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[9] = (uint8_t)(data & 0xFF);
  data = (int)(-p_can->w1 * 100.0f);
  p_frame[12] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[11] = (uint8_t)(data & 0xFF);
  data = (int)(p_can->w2 * 100.0f);
  p_frame[14] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  p_frame[13] = (uint8_t)(data & 0xFF);

  // 5. Checksum
  uint16_t crc16 = ModbusCRC16(p_frame, 0, 14);
  p_frame[15] = (uint8_t)(crc16 & 0xFF);        // Byte thấp
  p_frame[16] = (uint8_t)((crc16 >> 8) & 0xFF); // Byte cao

  // 6. End byte
  p_frame[17] = end_byte;

  // Truyền dữ liệu qua UART bằng DMA để tránh block CPU và xung đột Lock với ngắt nhận
  HAL_UART_Transmit_DMA(p_huart, p_frame, 18);
}

/*************************************************************************************************************************/
// Hàm xử lý dữ liệu VS frame
void ProcessVSFrame(void) {
  // 0x00: Chạy/Dừng, 0x01,0x02,0x03: SMC 1,2,3
  uint8_t data_group = vs_frame[1];
  uint8_t len = (vs_frame[2] == 0x00) ? SHORT_FRAME_LEN : LONG_FRAME_LEN;

  // Xử lý lệnh Chạy/Dừng
  if (data_group == 0x00) {
    if (vs_frame[3] == 0x01) {
      control = 1; // Lệnh chạy xe
    } else if (vs_frame[3] == 0x00) {
      control = 0; // Lệnh dừng xe
    }
  }

  // Xử lý cập nhật tham số (cập nhật trực tiếp vào struct control_smc)
  if (len == SHORT_FRAME_LEN) {
    // frame ngắn: Cập nhật 1 biến
    uint8_t index = vs_frame[3];
    int16_t raw_val = (int16_t)(((uint32_t)vs_frame[5] << 8) | vs_frame[4]);
    float real_val = (float)raw_val / 100.0f;

    if (data_group == 0x01) {
      switch (index) {
      case 0x00:
        control_smc.c1 = real_val;
        break;
      case 0x01:
        control_smc.ETA1 = real_val;
        break;
      case 0x02:
        control_smc.k1 = real_val;
        break;
      }
    } else if (data_group == 0x02) {
      switch (index) {
      case 0x00:
        control_smc.c2 = real_val;
        break;
      case 0x01:
        control_smc.ETA2 = real_val;
        break;
      case 0x02:
        control_smc.k2 = real_val;
        break;
      }
    } else if (data_group == 0x03) {
      switch (index) {
      case 0x00:
        control_smc.c3 = real_val;
        break;
      case 0x01:
        control_smc.ETA3 = real_val;
        break;
      case 0x02:
        control_smc.k3 = real_val;
        break;
      }
    }
  } else if (len == LONG_FRAME_LEN) {
    // frame dài: Cập nhật 3 biến cùng lúc
    float val_a = (float)(int16_t)(((uint32_t)vs_frame[4] << 8) | vs_frame[3]) / 100.0f;
    float val_beta = (float)(int16_t)(((uint32_t)vs_frame[6] << 8) | vs_frame[5]) / 100.0f;
    float val_k = (float)(int16_t)(((uint32_t)vs_frame[8] << 8) | vs_frame[7]) / 100.0f;

    if (data_group == 0x01) {
      control_smc.c1 = val_a;
      control_smc.ETA1 = val_beta;
      control_smc.k1 = val_k;
    } else if (data_group == 0x02) {
      control_smc.c2 = val_a;
      control_smc.ETA2 = val_beta;
      control_smc.k2 = val_k;
    } else if (data_group == 0x03) {
      control_smc.c3 = val_a;
      control_smc.ETA3 = val_beta;
      control_smc.k3 = val_k;
    }
  }
}

/*************************************************************************************************************************/
// Hàm đọc và xử lý dữ liệu trong VS Buffer
void ReadAndProcessVSData(void) {
  // Pop dữ liệu vào mảng đệm t_buff
  uint8_t t_buff[32];
  uint8_t len = vs_buffer[vs_tail].rlength;
  memcpy(t_buff, vs_buffer[vs_tail].buff, len);
  vs_tail = RingIncrease(vs_tail);

  // Đọc dữ liệu
  int8_t start_i = -1; // Index của start byte
  int8_t end_i = 0;    // Index của end byte
  uint8_t begin_i = 0; // Điểm bắt đầu của biến đếm i trong vòng lặp for
  while ((len - begin_i) >= SHORT_FRAME_LEN) {
    // Tìm start byte
    for (uint8_t i = begin_i; i < len; i++) {
      if (t_buff[i] == DOWNLINK_START_BYTE) {
        start_i = i;
        break;
      }
    }
    // Không tìm được start byte / Số byte còn lại quá ít -> Thoát hàm
    if (start_i == -1 || (len - start_i) < SHORT_FRAME_LEN) {
      return;
    }
    // Kiểm tra end byte
    uint8_t frame_len =
        (t_buff[start_i + 2] == 0x01 && (len - start_i) >= LONG_FRAME_LEN) ? LONG_FRAME_LEN : SHORT_FRAME_LEN;
    end_i = start_i + frame_len - 1;
    if (t_buff[end_i] == END_BYTE) {
      // Kiểm tra checksum
      uint16_t crc16 = ModbusCRC16(t_buff, start_i, (end_i - 3));
      uint8_t low_byte_crc = (uint8_t)(crc16 & 0xFF);
      uint8_t high_byte_crc = (uint8_t)((crc16 >> 8) & 0xFF);
      if (low_byte_crc == t_buff[end_i - 2] && high_byte_crc == t_buff[end_i - 1]) {
        // Frame đúng -> Xử lý dữ liệu
        memcpy(vs_frame, &t_buff[start_i], frame_len);
        ProcessVSFrame();
        begin_i = end_i + 1; // Tìm tiếp frame mới
        start_i = -1;
        continue;
      }
    }
    // Tìm start byte mới
    begin_i = start_i + 1;
    start_i = -1;
  }
}

#include "ESP32.h"
#include <stdint.h>
#include <string.h>

/******************************************************************************/
volatile uint8_t tx_esp32;          // Cờ báo cần gửi dửi liệu lên ESP32
uint8_t tx_frame[20];               // Truyền dữ liệu ra cổng UART6
uint8_t rx_frame[32];               // Nhận dữ liệu từ cổng UART6
uint8_t VS_flag;                    // Cờ báo đọc thành công 1 frame từ VS
uint8_t VS_frame[32];               // Pop dữ liệu từ VS Buffer
volatile uint8_t VS_head = 0;       // Con trỏ đầu của VS Ring Buffer
volatile uint8_t VS_tail = 0;       // Con trỏ đuôi của VS Ring Buffer
VS_struct VS_Buffer[RING_BUFF_LEN]; // Ring Buffer nhận dữ liệu VS từ cổng UART6

/******************************************************************************/
// Hàm tính mã Modbus CRC 16
uint16_t Modbus_CRC16(uint8_t *frame, uint8_t start_i,
                      uint8_t end_i) // start_i, end_i: là index đầu và cuối của
                                     // phần dữ liệu trong frame muốn tính CRC
{
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = start_i; pos <= end_i; pos++) {
    crc ^= (uint16_t)frame[pos];
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
/******************************************************************************/
// Hàm tạo và gửi dữ liệu lên ESP32
void Send_Data_To_ESP32(UART_HandleTypeDef *huart, HWT906 *hwt906_data,
                        SP_CAN *can, GET_CAN *get_can, uint8_t *frame) {

  uint8_t up_StartByte = 0xAA;
  uint8_t EndByte = 0xED;

  // 1. Start byte
  frame[0] = up_StartByte;

  // 2. Data
  int data = (int)(-(hwt906_data->angle.x) * 100.0f);
  frame[2] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[1] = (uint8_t)(data & 0xFF);
  data = (int)(hwt906_data->angle.z * 100.0f);
  frame[4] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[3] = (uint8_t)(data & 0xFF);
  data = (int)(hwt906_data->angular_velocity.x * 100.0f);
  frame[6] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[5] = (uint8_t)(data & 0xFF);
  data = (int)(hwt906_data->angular_velocity.z * 100.0f);
  frame[8] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[7] = (uint8_t)(data & 0xFF);
  data = (int)(-(get_can->w1 + get_can->w2) * 6.28f * (0.073f / 2.0f) * 50.0f);
  frame[10] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[9] = (uint8_t)(data & 0xFF);
  data = (int)(-can->w1 * 100.0f);
  frame[12] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[11] = (uint8_t)(data & 0xFF);
  data = (int)(can->w2 * 100.0f);
  frame[14] = (uint8_t)(((uint32_t)data >> 8) & 0xFF);
  frame[13] = (uint8_t)(data & 0xFF);

  // 5. Checksum
  uint16_t crc16 = Modbus_CRC16(frame, 0, 14);
  frame[15] = (uint8_t)(crc16 & 0xFF);        // Byte thấp
  frame[16] = (uint8_t)((crc16 >> 8) & 0xFF); // Byte cao

  // 6. End byte
  frame[17] = EndByte;

  // Trasmit frame via UART
  HAL_UART_Transmit(huart, frame, 18, 5);
}
/******************************************************************************/
// Hàm xử lý dữ liệu VS frame
void Process_VS_frame(void) {
  uint8_t data_group = VS_frame[1]; // 0x00: Chạy/Dừng, 0x01,0x02,0x03: SMC
                                    // 1,2,3
  uint8_t len = (VS_frame[2] == 0x00) ? short_frame_len : long_frame_len;

  // Xử lý lệnh Chạy/Dừng
  if (data_group == 0x00) {
    if (VS_frame[3] == 0x01) {
      control = 1; // Lệnh chạy xe
    } else if (VS_frame[3] == 0x00) {
      control = 0; // Lệnh dừng xe
    }
  }

  // Xử lý cập nhật tham số (cập nhật trực tiếp vào struct control_smc)
  if (len == short_frame_len) {
    // frame ngắn: Cập nhật 1 biến
    uint8_t index = VS_frame[3];
    int16_t raw_val = (int16_t)(((uint32_t)VS_frame[5] << 8) | VS_frame[4]);
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
  } else if (len == long_frame_len) {
    // frame dài: Cập nhật 3 biến cùng lúc
    float val_a =
        (float)(int16_t)(((uint32_t)VS_frame[4] << 8) | VS_frame[3]) / 100.0f;
    float val_beta =
        (float)(int16_t)(((uint32_t)VS_frame[6] << 8) | VS_frame[5]) / 100.0f;
    float val_k =
        (float)(int16_t)(((uint32_t)VS_frame[8] << 8) | VS_frame[7]) / 100.0f;

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
/******************************************************************************/
// Hàm đọc và xử lý dữ liệu trong VS Buffer
void Read_And_Process_VSData(void) {
  // Pop dữ liệu vào t_buff
  uint8_t t_buff[32];
  uint8_t len = VS_Buffer[VS_tail].rlength;
  if (len > 32)
    len = 32;
  memcpy(t_buff, VS_Buffer[VS_tail].buff, len);
  VS_tail = ring_increase(VS_tail);

  // Đọc dữ liệu
  int8_t start_i = -1; // Index của start byte
  int8_t end_i = 0;    // Index của end byte
  uint8_t begin_i =
      0; // Điểm bắt đầu của biến đếm i trong vòng lặp for
         // Lặp cho đến khi số phần tử còn lại ít hơn 1 short frame
  while ((len - begin_i) >= short_frame_len) {
    // Tìm start byte
    for (uint8_t i = begin_i; i < len; i++) {
      if (t_buff[i] == VS_Start_Byte) {
        start_i = i;
        break;
      }
    }
    // Không tìm được start byte / Số byte còn lại quá ít -> Thoát hàm
    if (start_i == -1 || (len - start_i) < short_frame_len) {
      return;
    }
    // Kiểm tra end byte
    uint8_t frame_len =
        (t_buff[start_i + 2] == 0x01 && (len - start_i) >= long_frame_len)
            ? long_frame_len
            : short_frame_len;
    end_i = start_i + frame_len - 1;
    if (t_buff[end_i] == VS_End_Byte) {
      // Kiểm tra checksum
      uint16_t crc16 = Modbus_CRC16(t_buff, start_i, (end_i - 3));
      uint8_t low_byte_crc = (uint8_t)(crc16 & 0xFF);
      uint8_t high_byte_crc = (uint8_t)((crc16 >> 8) & 0xFF);
      if (low_byte_crc == t_buff[end_i - 2] &&
          high_byte_crc == t_buff[end_i - 1]) {
        // Frame đúng -> Xử lý dữ liệu
        memcpy(VS_frame, &t_buff[start_i], frame_len);
        Process_VS_frame();
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

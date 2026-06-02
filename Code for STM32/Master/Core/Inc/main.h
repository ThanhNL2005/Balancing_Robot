/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h" // IWYU pragma: export

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/**********************************************************************************************/

/**********************************************************************************************/
typedef enum { ACC, RUN, DEC } RAMP_STATE;
/**********************************************************************************************/
// Ramp Fcn generator
typedef struct {
  float f_SPk; // Current and past value of setpoint
  float f_inputMax, f_inputMin;
  uint16_t ui16_T_Acc, ui16_T_Dec;           // Ramp up and down time (s)
  float f_delta_Acc, f_delta_Dec, f_delta;   // step size in ACC and DEC mode
  uint16_t ui16_set_AccCnt, ui16_set_DecCnt; // Set count - 10ms Resolution
  RAMP_STATE enum_State;
  float f_Output;
} RAMP;
/**********************************************************************************************/
// PI Controller
typedef struct {
  float Kp, TI, Ki, Kd;                   // Controller parameters
  float Ks, D;                            // Ks: Anti-windup Gain; D = 1-Ts/TI
  float Umax, Umin, Usk, Usk_1, Uk, Uk_1; // Control signal
  float Ek, Ek_1, Ek_2, Esk, Esk_1;       // Error
  float Ts;                               // Sampling time
  uint8_t ui8_MODE;                       // Mode = 0: P, Mode = 1: PI
} cPI;
// PI Controller
typedef struct {
  float Kp, N, Ki, Kd; // Controller parameters
  float T;
  float alpha, beta;                                  // ph?c v? t�nh h? s?
  float b0, b1, b2;                                   // h? s? s?
  float Umax, Umin, Uk, Uk_1, UP, UI, UI_1, UD, UD_1; // Control signal
  float Ek, Ek_1, Ek_2, Esk, Esk_1;                   // Error
  float Ts;                                           // Sampling time
  uint8_t ui8_MODE; // Mode = 0: P, Mode = 1: PI
} cPD;
/**********************************************************************************************/
// Second-Order Unity-Gain Low Pass Filter
typedef struct {
  float Ts;             // Sampling time
  float Fc;             // Conrner frequency (Hz)
  float a1, a2, b1, b2; // Parameters of the filter
  float alpha;
  float yk, yk_1, yk_2, uk, uk_1;
} LPF;
/**********************************************************************************************/
// usart data
typedef struct {
  // Buffer
  uint8_t rxBuff[100];
  uint8_t txBuff[100];
  // Header
  uint8_t frameLength, groupID, senderAddr, receiverAddr, messType, dataLength;
  uint16_t messCount;
  // Data payload

  // State flow
  uint8_t STATE;
  uint8_t RxFlag;
  uint8_t rxByte, rxPointer;
  uint16_t timeOut;
} usartData;
/**********************************************************************************************/

/**********************************************************************************************/

// HWT906
/* data HWT906*/
typedef struct {
  // Tọa đô
  struct {
    float x;
    float y;
    float z;
  } acceleration, angular_velocity, angle;
  // Trạng thái
  struct {
    uint8_t DONE;
    uint8_t ERROR;
  } STATE;
} HWT906;

typedef struct {
  float thetaSP;
  float theta;
} THETA;

// CAN
typedef struct {
  float w1; // Vận tốc góc bánh 1
  float w2; // Vận tốc góc bánh 2
  uint8_t data[8];
} SP_CAN, GET_CAN;

// Filter
typedef struct {
  float dt;
  float x0;                 // angle
  float x1;                 // velocity angle
  float p00, p01, p10, p11; // matrix P
  float q00, q11;           // matrix Q
  float r00, r11;           // matrix R
} KA_Filter;
// HSMC
typedef struct {
  float mB; // khối lượng thân con lắc
  float l;  // chiều dài con lắc
  float r;  // bán kính bánh xe
  float mW; // khối lượng mỗi bánh xe
  float d;  // khoảng cách giữa hai bánh xe
  float g;  // gia tốc trọng trường
  float I1; // moment quán tính của thân con lắc quanh trục d
  float I2; // moment quán tính của thân con lắc quanh trục l
  float I3; // moment quán tính của thân con lắc quanh trục d
  float J;  // moment quán tính của mỗi bánh xe
  float K;  // moment quán tính bổ sung của mỗi bánh xe
} TWBMR;
typedef struct {
  float f1;
  float f2;
  float f3;
} F;
typedef struct {
  float g1;
  float g2;
  float g3;
} G;
typedef struct {
  // parameter system - Thông số hệ thống
  float x1;
  float x2;
  float x3;
  float x4;
  float x5;
  float x6;

  // parameter control - Thông số bộ điều khiển
  float c1, c2, c3; // a1, a2, a3
  float k1, k2, k3;
  float ETA1, ETA2, ETA3; // beta1, beta2, beta3
  float lambda1;
  float beta1;
  float K3;

  // error
  float e1;
  float e2;
  float e3;
  float e4;
  float e5;
  float e6;

  // sliding face
  float s1;
  float s2;
  float s3;
  float s;

  // output
  float ueq1;
  float ueq2;
  float usw;
  float u_x, u_theta, u_psi;
  float u1;
  float u2;
} HSMC, SMC;
// ESP32
typedef struct {
  uint8_t rx;
  uint8_t tx;
} ESP32;
/**********************************************************************************************/
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern UART_HandleTypeDef huart6;

// CONTROL
extern volatile uint8_t control;
void control_theta_pid(void);
void control__pid(void);
void set_PID(cPI *pPID, float kp, float ki, float kd, float n, float ts,
             float umax, float umin);
float PID_control(cPI *pPID, float setpoint, float measure);
void vPI_Init(cPI *pPI, float TI, float Lambda, float Ts, float MaxSP,
              float Usat);
float fPI_Run(cPI *pPI, float Ref, float Fb);
void PID_init(cPD *pPID, uint8_t mode, float Ts, float K_p, float K_i,
              float K_d, float N, float max, float min);
float PID_run(cPD *pPID, float setpoin, float measure);

// CAN
void FDCAN_Config(void);
void send_via_can(SP_CAN *can_send);

// FILTER
void KA_init(KA_Filter *kf, float dt, float q00, float q11, float r00,
             float r11, float angle0, float rate0);
float KA_update(KA_Filter *kf, float angle_meas, float gyro_meas);
// HWT906
void angle_referenc(void);
void restart_HWT906(void);
void read_frame(uint8_t *buffer, uint8_t size);
void rearrange_frame(const uint8_t *dmaBuffer, uint8_t *orderedBuffer);
void process_frame(HWT906 *hwt906_data, uint8_t *data);

// HSMC
void TwoWBMR_Init(TWBMR *robot);
void HSMC_init(HSMC *state);
void HSMC_control(TWBMR *param, HSMC *hsmc, F *f, G *g);
void SMC_control(TWBMR *param, SMC *smc, F *f, G *g);
extern HSMC control_smc;
extern TWBMR parameter_TWBMR;
extern HSMC control_smc;
extern F f_funtion;
extern G g_funtion;
// sin/cos
#define STEP 0.00017453
#define TABLE_SIZE 36000

extern float sin_table[TABLE_SIZE];
extern float cos_table[TABLE_SIZE];

void init_lookup_table(void);
float lookup_sin(float radian);
float lookup_cos(float radian);
// RS485
void send_rs485(uint8_t sender, uint8_t receiver, float f1, float f2,
                uint8_t *frame);

/******************************************************************************/
void Toggle_Led(uint8_t led_number);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_3
#define LED2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

#define PI 3.1416
#define NumOfLPF 20

// NEXTION Return Data

#define RED 63488
#define YELLOW 65504
#define GREEN 2016
#define PURPLE 53594

#define RS485_Port 4
#define PC_Port 5

#define P 0
#define P_I 1
#define P_I_D 2

#define run_stop 0x00
#define PID_x 0x01
#define PID_theta 0x02
#define PID_psi 0x03
#define hsmc_e 0x04
#define hsmc_d 0x05
#define hsmc_c 0x06
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

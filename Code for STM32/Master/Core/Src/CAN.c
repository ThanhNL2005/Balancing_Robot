#include <stdio.h>
#include "main.h"
#include <string.h>
#include "stm32h7xx_hal.h"           // Include STM32 HAL library for H7 series
#include "stm32h7xx_hal_fdcan.h"     // Include STM32 HAL FDCAN library

// Define constants
#define ROOT_2_OVER_2 0.70710678118f  // Approximation for sqrt(2) / 2
#define DISTANCE_D 1.0f   // Set distance from robot center to each wheel, modify as per your robot
#define a 0.2f // Half distance of robot length 
#define b 0.2f // Half distance of robot width
#define WHEEL_RADIUS 0.1f             // Radius of each wheel (m), modify based on your robot

// Declaration of FDCAN handler defined globally in one file
extern FDCAN_TxHeaderTypeDef TxHeader;
extern FDCAN_RxHeaderTypeDef RxHeader;
extern FDCAN_HandleTypeDef hfdcan2;
// define void
extern GET_CAN can_get;
extern void Error_Handler(void);
uint8_t Can_RxData[8];
double test_w1[2000];
double test_w2[2000];
int size, size_2;
uint32_t RxLocation;

// Function to initialize FDCAN communication
void FDCAN_Config(void)
{
	FDCAN_FilterTypeDef sFilterConfig;
	/* Configure Rx filter */
	sFilterConfig.IdType = FDCAN_STANDARD_ID; // config enable stand_mode(11 byte)
	sFilterConfig.FilterIndex = 0;
	sFilterConfig.FilterType = FDCAN_FILTER_RANGE;        // chose type filter
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; // choonse type ram Buffer (FIFO0, FIFO1)
	sFilterConfig.FilterID1 = 0x09;   // config ID of tranmitter
	sFilterConfig.FilterID2 = 0x15;      // config 
	sFilterConfig.RxBufferIndex = 8; // the length of the frame that will be received

	/* Start the FDCAN module */
	if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK)
	{
		//    Error_Handler();
	}

	if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
	{
		Error_Handler();
	}
	/* Prepare Tx Header */
	TxHeader.IdType = FDCAN_STANDARD_ID; // Specifies the identifier.
	TxHeader.TxFrameType = FDCAN_DATA_FRAME; //the frame type of the message that will be transmitted
	TxHeader.DataLength = FDCAN_DLC_BYTES_8; //the length of the frame that will be transmitted.
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE; //the error state indicator.
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF; //whether the Tx frame will be transmitted with or without bit rate switching.
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN; // the Tx frame will be transmitted in classic
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS; //the event FIFO control.
	TxHeader.MessageMarker = 0;
}

// Function to send data via FDCAN to the STM32 slaves
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
	{
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, Can_RxData) == HAL_OK)
		{
			union
			{
				double d;
				uint8_t bytes[8];
			} double_to_byte;
			for (int i = 0; i < 8; i++)
			{
				double_to_byte.bytes[i] = Can_RxData[i];
			}
			if (RxHeader.Identifier == 0x11)
			{
				can_get.w1 = -double_to_byte.d;
			}
			if (RxHeader.Identifier == 0x12)
			{
				can_get.w2 = double_to_byte.d;
			}
			Toggle_Led(2);
		}
	}
}

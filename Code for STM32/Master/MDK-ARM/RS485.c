#include "main.h"
#include <string.h> 

#define FRAME_START_BYTE 0xAA
#define FRAME_SIZE 12

extern UART_HandleTypeDef huart2;

// Hàm tính checksum:
uint8_t calculate_checksum(uint8_t *data, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}
void float_to_bytes(float value, uint8_t *bytes) {
    typedef union {
        float    f;
        uint8_t  b[4];
    } FloatUnion;
    
    FloatUnion u;
    u.f = value;
    bytes[0] = u.b[0];
    bytes[1] = u.b[1];
    bytes[2] = u.b[2];
    bytes[3] = u.b[3];
}
float bytes_to_float(const uint8_t *bytes) {
    typedef union {
        float    f;
        uint8_t  b[4];
    } FloatUnion;
    
    FloatUnion u;
    u.b[0] = bytes[0];
    u.b[1] = bytes[1];
    u.b[2] = bytes[2];
    u.b[3] = bytes[3];
    return u.f;
}
// [start (1 byte)][addr_sender (1 byte)][addr_receiver (1 byte)][data: 2 bi?n float (8 byte)][checksum (1 byte)]
void create_frame(uint8_t sender, uint8_t receiver, float f1, float f2, uint8_t *frame) {
    // start byte
    frame[0] = FRAME_START_BYTE;
    
    // setup address send and receive
    frame[1] = sender;
    frame[2] = receiver;
    
    // convert 2 variable float to byte
    float_to_bytes(f1, &frame[3]);  
    float_to_bytes(f2, &frame[7]); 
    
    // caculator checksum
    frame[11] = calculate_checksum(frame, 11);
}
void send_rs485(uint8_t sender, uint8_t receiver, float f1, float f2,uint8_t *frame){
	    create_frame(sender,receiver,f1,f2,frame);
      HAL_UART_Transmit_DMA(&huart2, frame, FRAME_SIZE);
}

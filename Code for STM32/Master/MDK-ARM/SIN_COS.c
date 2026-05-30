#include <stdio.h>
#include <math.h>
#include "main.h"


void init_lookup_table(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        float rad = i * STEP; // Chuy?n index thành giá tr? radian
        sin_table[i] = sinf(rad);
        cos_table[i] = cosf(rad);
    }
}

// Hàm tra c?u sin t? radian
float lookup_sin(float radian) {
    int index = (int)(radian / STEP) % TABLE_SIZE;
    if (index < 0) index += TABLE_SIZE;
    return sin_table[index];
}

// Hàm tra c?u cos t? radian
float lookup_cos(float radian) {
    int index = (int)(radian / STEP) % TABLE_SIZE;
    if (index < 0) index += TABLE_SIZE;
    return cos_table[index];
}


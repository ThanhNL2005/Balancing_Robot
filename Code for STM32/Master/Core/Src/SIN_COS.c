#include <stdio.h>
#include <math.h>
#include "main.h"

float sin_table[TABLE_SIZE];
float cos_table[TABLE_SIZE];

// Khởi tạo bảng tra
void init_lookup_table(void)
    {
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        float rad = i * STEP; // Chuyển index thành giá trị radian
        sin_table[i] = sinf(rad);
        cos_table[i] = cosf(rad);
    }
}

// Hàm tra cứu sin theo radian
float lookup_sin(float radian)
{
    int index = (int)(radian / STEP) % TABLE_SIZE; // Chuyển giá trị radian thành index
    if (index < 0) index += TABLE_SIZE;
    return sin_table[index];
}

// Hàm tra cứu cos theo radian
float lookup_cos(float radian)
{
    int index = (int)(radian / STEP) % TABLE_SIZE; // Chuyển giá trị radian thành index
    if (index < 0) index += TABLE_SIZE;
    return cos_table[index];
}

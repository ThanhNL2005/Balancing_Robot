#include "main.h"
#include <stdio.h>
#define N 10      // S? lu?ng m?u do

void KA_init(KA_Filter *kf, float dt,
             float q00, float q11,
             float r00, float r11,
             float angle0, float rate0)
{
    // Gán thông s?
    kf->dt = dt;
    
    // Kh?i t?o tr?ng thái
    kf->x0 = angle0;  // góc
    kf->x1 = rate0;   // t?c d? góc
    
    // Kh?i t?o ma tr?n sai s? P (don v?)
    kf->p00 = 1.0f;
    kf->p01 = 0.0f;
    kf->p10 = 0.0f;
    kf->p11 = 1.0f;
    
    // Kh?i t?o nhi?u quá trình
    kf->q00 = q00;
    kf->q11 = q11;
    
    // Kh?i t?o nhi?u do
    kf->r00 = r00;
    kf->r11 = r11;
}

/**
 * @brief Hàm c?p nh?t b? l?c Kalman cho m?t c?p do m?i (góc và t?c d? góc)
 * @param kf         : Con tr? d?n bi?n ki?u KA_Filter
 * @param angle_meas : Giá tr? góc do du?c t? c?m bi?n
 * @param gyro_meas  : Giá tr? t?c d? góc do du?c t? gyro
 * @return Góc dã du?c l?c (kf->x0 sau khi c?p nh?t)
 */
float KA_update(KA_Filter *kf, float angle_meas, float gyro_meas)
{
    // Bu?c d? doán (Predict)
    float x0_pred = kf->x0 + kf->x1 * kf->dt;  // F = [[1, dt],[0,1]]
    float x1_pred = kf->x1;
    
    // Tính P_pred = F * P * F^T + Q
    float p00_pred = kf->p00 + (kf->p01 + kf->p10)*kf->dt + kf->p11*(kf->dt*kf->dt) + kf->q00;
    float p01_pred = kf->p01 + kf->p11 * kf->dt;
    float p10_pred = kf->p10 + kf->p11 * kf->dt;
    float p11_pred = kf->p11 + kf->q11;
    
    // Tính S = P_pred + R (vì H = I, 2x2)
    float s00 = p00_pred + kf->r00;
    float s01 = p01_pred;
    float s10 = p10_pred;
    float s11 = p11_pred + kf->r11;
    
    // Ð?nh th?c c?a S
    float detS = s00 * s11 - s01 * s10;
    
    // Ma tr?n ngh?ch d?o c?a S
    float invS00 =  s11 / detS;
    float invS01 = -s01 / detS;
    float invS10 = -s10 / detS;
    float invS11 =  s00 / detS;
    
    // Kalman Gain K = P_pred * invS
    float k00 = p00_pred * invS00 + p01_pred * invS10;
    float k01 = p00_pred * invS01 + p01_pred * invS11;
    float k10 = p10_pred * invS00 + p11_pred * invS10;
    float k11 = p10_pred * invS01 + p11_pred * invS11;
    
    // Sai s? do
    float y0 = angle_meas - x0_pred;
    float y1 = gyro_meas  - x1_pred;
    
    // C?p nh?t tr?ng thái
    kf->x0 = x0_pred + (k00 * y0 + k01 * y1);
    kf->x1 = x1_pred + (k10 * y0 + k11 * y1);
    
    // (I - K)
    float i00 = 1.0f - k00;
    float i01 = -k01;
    float i10 = -k10;
    float i11 = 1.0f - k11;
    
    // C?p nh?t ma tr?n P
    float p00_new = i00 * p00_pred + i01 * p10_pred;
    float p01_new = i00 * p01_pred + i01 * p11_pred;
    float p10_new = i10 * p00_pred + i11 * p10_pred;
    float p11_new = i10 * p01_pred + i11 * p11_pred;
    
    kf->p00 = p00_new;
    kf->p01 = p01_new;
    kf->p10 = p10_new;
    kf->p11 = p11_new;
    
    // Tr? v? góc dã l?c
    return kf->x0;
}

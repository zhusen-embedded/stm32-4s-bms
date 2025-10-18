#ifndef CELL_VOLTAGE_H
#define CELL_VOLTAGE_H

#include <stdint.h>

/* 分压 + 运放跟随进 ADC，再做两点校准。
 * 校准系数存 Flash，上电读出来。
 */

float cell_voltage(uint8_t ch);
void  cell_sample_all(float *out, uint8_t n);
void  cell_calib(float gain, float offset);

#endif

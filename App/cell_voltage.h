#ifndef CELL_VOLTAGE_H
#define CELL_VOLTAGE_H

#include <stdint.h>
#include "filter.h"

/* 分压 + 运放跟随进 ADC，再做两点校准。
 * 校准系数存 Flash，上电读出来。
 *
 * 纯逻辑部分（raw -> 电压）与 HAL 解耦，方便 host 单测。
 */

#define CELL_NUM        4
#define CELL_DIV_RATIO  4.0f        /* 1:4 分压 */
#define CELL_VREF       3.3f
#define CELL_ADC_FULL   4096.0f

typedef struct {
    float gain;
    float offset;
} cell_calib_t;

/* 单点换算：raw ADC -> 单体电压（V） */
float cell_raw_to_volt(uint16_t raw, const cell_calib_t *cal);

/* 中值滤波 + 换算，一步到位 */
float cell_read_filtered(uint16_t raw, med_filter_t *f, const cell_calib_t *cal);

void  cell_calib_set(cell_calib_t *cal, float gain, float offset);

/* 用两个标定点反算 gain/offset：
 *   v_true = v_meas * gain + offset
 * 传入两点的实测值与真值，解二元一次方程。 */
void  cell_calib_solve(cell_calib_t *cal,
                       float v_meas_lo, float v_true_lo,
                       float v_meas_hi, float v_true_hi);

#endif

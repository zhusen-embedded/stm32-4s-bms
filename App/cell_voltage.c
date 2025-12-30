#include "cell_voltage.h"

float cell_raw_to_volt(uint16_t raw, const cell_calib_t *cal)
{
    float v = (float)raw * CELL_VREF / CELL_ADC_FULL * CELL_DIV_RATIO;
    if (cal) v = v * cal->gain + cal->offset;
    return v;
}

float cell_read_filtered(uint16_t raw, med_filter_t *f, const cell_calib_t *cal)
{
    med_push(f, raw);
    return cell_raw_to_volt(med_get(f), cal);
}

void cell_calib_set(cell_calib_t *cal, float gain, float offset)
{
    if (cal == 0) return;
    cal->gain = gain;
    cal->offset = offset;
}

void cell_calib_solve(cell_calib_t *cal,
                      float v_meas_lo, float v_true_lo,
                      float v_meas_hi, float v_true_hi)
{
    if (cal == 0) return;

    float d = v_meas_hi - v_meas_lo;
    /* 两点重合解不出来，保持默认系数 */
    if (d > -1e-6f && d < 1e-6f) return;

    cal->gain = (v_true_hi - v_true_lo) / d;
    cal->offset = v_true_lo - v_meas_lo * cal->gain;
}

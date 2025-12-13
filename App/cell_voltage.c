#include "cell_voltage.h"
#include "bsp_adc.h"

// 分压比 1:4，参考 3.3 V / 4096
#define DIV_RATIO   4.0f
#define VREF        3.3f
#define ADC_FULL    4096.0f

static float s_gain = 1.0f;
static float s_offset = 0.0f;

float cell_voltage(uint8_t ch)
{
  float v = bsp_adc_raw(ch) * VREF / ADC_FULL * DIV_RATIO;
  return v * s_gain + s_offset;
}

void cell_calib(float gain, float offset)
{
  s_gain = gain;
  s_offset = offset;
}


void cell_sample_all(float *out, uint8_t n)
{
    for (uint8_t i = 0; i < n; i++) {
        out[i] = cell_voltage(i);
    }
}

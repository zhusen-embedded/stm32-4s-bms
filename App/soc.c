#include "soc.h"

#define CAP_AH   10.0f

static float s_soc = 100.0f;
static uint32_t s_rest_ms = 0;

// 4 串单体 OCV-SOC 表，步进 5%
static const float ocv_tbl[] = {
  3.00f, 3.45f, 3.55f, 3.62f, 3.66f, 3.69f, 3.71f,
  3.73f, 3.76f, 3.79f, 3.83f, 3.88f, 3.94f, 4.00f,
  4.05f, 4.10f, 4.15f, 4.18f, 4.20f, 4.20f, 4.20f
};

static float ocv_to_soc(float v)
{
  uint8_t i;
  for (i = 0; i < 20; i++) {
    if (v < ocv_tbl[i + 1]) {
      float span = ocv_tbl[i + 1] - ocv_tbl[i];
      return (i + (v - ocv_tbl[i]) / span) * 5.0f;
    }
  }
  return 100.0f;
}

void soc_update(float current_a, float dt_s)
{
  s_soc -= current_a * dt_s / 3600.0f / CAP_AH * 100.0f;
  if (s_soc > 100.0f) s_soc = 100.0f;
  if (s_soc < 0.0f)   s_soc = 0.0f;
}

// 静置且电流小于 50 mA 时用 OCV 拉回真值
void soc_rest_correct(float cell_v, float current_a)
{
  if (current_a < 0.05f && current_a > -0.05f) {
    s_rest_ms += 1000;
    if (s_rest_ms > 30u * 60u * 1000u) {
      s_soc = ocv_to_soc(cell_v);
      s_rest_ms = 0;
    }
  } else {
    s_rest_ms = 0;
  }
}

float soc_get(void) { return s_soc; }


void soc_reset(void)
{
    s_soc = 100.0f;
    s_rest_ms = 0;
}

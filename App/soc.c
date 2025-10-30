#include "soc.h"

#define CAP_AH   10.0f   // 标称 10 Ah

static float s_soc = 100.0f;

void soc_update(float current_a, float dt_s)
{
  // 放电为正，充电为负
  s_soc -= current_a * dt_s / 3600.0f / CAP_AH * 100.0f;
  if (s_soc > 100.0f) s_soc = 100.0f;
  if (s_soc < 0.0f)   s_soc = 0.0f;
}

float soc_get(void) { return s_soc; }

#include "balance.h"

#define BAL_DIFF_MV   30    // 压差超过 30 mV 才开均衡
#define BAL_SETTLE_MS 10    // 关均衡后等 10 ms 再采样

static uint8_t s_mask;

void balance_update(const float *cells, uint8_t n)
{
  float min_v = cells[0];
  uint8_t i;

  for (i = 1; i < n; i++) if (cells[i] < min_v) min_v = cells[i];

  s_mask = 0;
  for (i = 0; i < n; i++) {
    if ((cells[i] - min_v) * 1000.0f > BAL_DIFF_MV) s_mask |= (1u << i);
  }
  balance_apply(s_mask);
}

void balance_apply(uint8_t mask)
{
  HAL_GPIO_WritePin(BAL_PORT, 0x0F, (GPIO_PinState)(~mask & 0x0F));
}

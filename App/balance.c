#include "balance.h"

#define BAL_DIFF_MV   30
#define BAL_SETTLE_MS 10

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
}

// 采样前先关均衡，等泄放电流消失后再读，避免分压点被拉偏
void balance_pre_sample(void)
{
  balance_apply(0);
  HAL_Delay(BAL_SETTLE_MS);
}

void balance_post_sample(void)
{
  balance_apply(s_mask);
}


void balance_apply(uint8_t mask)
{
    /* 均衡引脚是 P1.0~P1.3，低电平导通泄放 */
    GPIOA->BSRR = (uint32_t)(~mask & 0x0F) << 16;
}

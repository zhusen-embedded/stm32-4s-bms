#include "balance.h"

#include "bsp_balance.h"
#include "delay.h"

static uint8_t s_mask;

uint8_t balance_mask(const float *cells, uint8_t n)
{
    if (cells == 0 || n == 0) return 0;
    if (n > 8) n = 8;

    float min_v = cells[0];
    for (uint8_t i = 1; i < n; i++) {
        if (cells[i] < min_v) min_v = cells[i];
    }

    uint8_t mask = 0;
    for (uint8_t i = 0; i < n; i++) {
        if ((cells[i] - min_v) * 1000.0f > (float)BAL_DIFF_MV) {
            mask |= (uint8_t)(1u << i);
        }
    }
    return mask;
}

void balance_set_mask(uint8_t mask)
{
    s_mask = mask;
    bsp_balance_apply(mask);
}

uint8_t balance_get_mask(void)
{
    return s_mask;
}

/* 采样前先关均衡，等泄放电流消失后再读，避免分压点被拉偏。
 * 原实现调了 balance_apply(0) 但那个函数只写 BSRR 复位位、
 * 从来没把引脚置高，等于均衡永远没开过。 */
void balance_pre_sample(void)
{
    bsp_balance_apply(0);
    delay_ms(BAL_SETTLE_MS);
}

void balance_post_sample(void)
{
    bsp_balance_apply(s_mask);
}

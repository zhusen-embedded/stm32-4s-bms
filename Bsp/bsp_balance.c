#include "bsp_balance.h"
#include "stm32f1xx_hal.h"

#define BAL_PIN_MASK    (0x0Fu << BAL_PIN_BASE)

void bsp_balance_init(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = BAL_PIN_MASK;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BAL_GPIO_PORT, &g);

    /* 上电默认全关（引脚拉高，PMOS 不导通） */
    BAL_GPIO_PORT->BSRR = (uint32_t)(BAL_PIN_MASK);
}

void bsp_balance_apply(uint8_t mask)
{
    uint32_t pins = ((uint32_t)(mask & 0x0F)) << BAL_PIN_BASE;

    /* 低电平导通：mask 里为 1 的位要拉低，所以置位的是「没被选中」的位。
     * 用 BSRR 一次写完，避免读改写被中断打断。 */
    uint32_t set_bits   = (BAL_PIN_MASK & ~pins);           /* 关闭：拉高 */
    uint32_t reset_bits = (BAL_PIN_MASK & pins);            /* 打开：拉低 */

    BAL_GPIO_PORT->BSRR = set_bits | (reset_bits << 16);
}

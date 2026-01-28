#ifndef BSP_BALANCE_H
#define BSP_BALANCE_H

#include <stdint.h>

/* 均衡引脚驱动。4 节对应 PB0~PB3，低电平导通泄放（PMOS 栅极拉低）。
 *
 * 关键：BSRR 的高低 16 位分别是「置位」和「复位」。
 *   置 1  -> BSRR 低 16 位对应 bit
 *   清 0  -> BSRR 高 16 位对应 bit
 * 原实现只写了复位位，引脚从来不会变高，均衡永远关着。
 */

#define BAL_GPIO_PORT   GPIOB
#define BAL_PIN_BASE    0

void bsp_balance_init(void);
void bsp_balance_apply(uint8_t mask);

#endif

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/* 毫秒/微秒延时。基于 SysTick 的 HAL_Delay 精度够用（1 ms 节拍），
 * 微秒级用 DWT 周期计数器。 */

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

/* 简单软时钟，1 ms 递增，由 SysTick 中断调用 */
void     tick_inc(void);
uint32_t tick_ms(void);

#endif

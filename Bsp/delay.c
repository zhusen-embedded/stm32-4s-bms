#include "delay.h"
#include "stm32f1xx_hal.h"

static volatile uint32_t s_tick;

void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void delay_us(uint32_t us)
{
    /* 72 MHz 下每次循环约 1 us，编译器优化等级变化会影响精度，
     * 这里的用途是 I2C/传感器时序，微秒级误差可接受。 */
    uint32_t n = us * (SystemCoreClock / 1000000u) / 4u;
    while (n--) {
        __NOP();
    }
}

void tick_inc(void)
{
    s_tick++;
}

uint32_t tick_ms(void)
{
    return s_tick;
}

/* HAL 的 SysTick 回调，1 kHz 节拍 */
void HAL_SYSTICK_Callback(void)
{
    tick_inc();
}

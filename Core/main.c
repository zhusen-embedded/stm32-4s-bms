#include <stdint.h>
#include "stm32f1xx_hal.h"

#include "bsp_adc.h"
#include "bsp_ina226.h"
#include "cell_voltage.h"
#include "soc.h"
#include "balance.h"
#include "protect.h"

/* 主循环三个节拍：
 *   10 ms  保护判定（异常 10 ms 内断 MOS）
 *   100 ms CAN 上报
 *   1 s    采样 + SOC + 均衡
 *
 * 不用 RTOS，三个节拍靠时间戳判断，逻辑简单、时序可预测。
 */

#define CELL_NUM        4
#define TICK_PROTECT_MS 10
#define TICK_CAN_MS     100
#define TICK_SAMPLE_MS  1000

extern void can_send_status(uint8_t board, float vpack, float cur,
                            float temp, float soc, uint16_t fault);

static float s_cells[CELL_NUM];
static uint32_t s_t_protect, s_t_can, s_t_sample;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_CAN1_Init();

    bsp_adc_init();
    bsp_ina226_init();
    bsp_can_init();

    soc_reset();

    while (1) {
        uint32_t now = HAL_GetTick();

        /* 10 ms：保护判定。用 if 而不是定时器中断，
         * 是为了让保护逻辑和采样在同一上下文，避免共享变量加锁。
         */
        if (now - s_t_protect >= TICK_PROTECT_MS) {
            s_t_protect = now;
            protect_check(s_cells, CELL_NUM, ina226_current(), ntc_temp());
        }

        /* 100 ms：CAN 上报总压、电流、温度、SOC、故障码 */
        if (now - s_t_can >= TICK_CAN_MS) {
            s_t_can = now;
            can_send_status(1,
                            ina226_bus_voltage(),
                            ina226_current(),
                            ntc_temp(),
                            soc_get(),
                            protect_fault_code());
        }

        /* 1 s：采样前先关均衡等 10 ms，避免分压点被泄放电流拉偏 */
        if (now - s_t_sample >= TICK_SAMPLE_MS) {
            s_t_sample = now;

            balance_pre_sample();
            cell_sample_all(s_cells, CELL_NUM);
            balance_post_sample();

            soc_update(ina226_current(), 1.0f);
            soc_rest_correct(s_cells[0], ina226_current());
            balance_update(s_cells, CELL_NUM);
        }
    }
}

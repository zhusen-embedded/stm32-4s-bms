#include <stdint.h>
#include "stm32f1xx_hal.h"

#include "bsp_adc.h"
#include "bsp_ina226.h"
#include "bsp_can.h"
#include "bsp_balance.h"
#include "bsp_ntc.h"
#include "delay.h"
#include "cell_voltage.h"
#include "soc.h"
#include "balance.h"
#include "protect.h"
#include "filter.h"

/* 主循环三个节拍：
 *   10 ms  保护判定（异常 10 ms 内断 MOS）
 *   100 ms CAN 上报
 *   1 s    采样 + SOC + 均衡
 *
 * 不用 RTOS，三个节拍靠时间戳判断，逻辑简单、时序可预测。
 */

#define CELL_NUM_ACTUAL 4
#define TICK_PROTECT_MS 10
#define TICK_CAN_MS     100
#define TICK_SAMPLE_MS  1000

#define MOS_PORT        GPIOA
#define MOS_PIN         GPIO_PIN_8      /* 主回路 MOS 使能，低电平断开 */

static float s_cells[CELL_NUM_ACTUAL];
static med_filter_t s_filt[CELL_NUM_ACTUAL];
static cell_calib_t s_cal[CELL_NUM_ACTUAL];

static uint32_t s_t_protect, s_t_can, s_t_sample;
static uint8_t  s_board_id = 1;

static void mos_set(uint8_t on)
{
    /* 低电平断开：故障时把引脚拉高 */
    HAL_GPIO_WritePin(MOS_PORT, MOS_PIN, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void sample_cells(void)
{
    for (uint8_t i = 0; i < CELL_NUM_ACTUAL; i++) {
        s_cells[i] = cell_read_filtered(bsp_adc_raw(i), &s_filt[i], &s_cal[i]);
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_CAN1_Init();

    bsp_adc_init();
    bsp_balance_init();
    bsp_ina226_init();
    bsp_can_init();
    bsp_ntc_init();

    for (uint8_t i = 0; i < CELL_NUM_ACTUAL; i++) {
        med_init(&s_filt[i]);
        cell_calib_set(&s_cal[i], 1.0f, 0.0f);
    }

    soc_reset();
    mos_set(1);

    while (1) {
        uint32_t now = HAL_GetTick();

        /* 10 ms：保护判定。用 if 而不是定时器中断，
         * 是为了让保护逻辑和采样在同一上下文，避免共享变量加锁。 */
        if (now - s_t_protect >= TICK_PROTECT_MS) {
            s_t_protect = now;

            prot_state_t st = protect_eval(s_cells, CELL_NUM_ACTUAL,
                                           ina226_current(), ntc_get_temp());
            if (st != PROT_OK) {
                /* 10 ms 内断主回路。故障码由 protect 内部锁存。 */
                mos_set(0);
            }
        }

        /* 100 ms：CAN 上报总压、电流、温度、SOC、故障码 */
        if (now - s_t_can >= TICK_CAN_MS) {
            s_t_can = now;

            can_status_t cs = {
                .vpack   = ina226_bus_voltage(),
                .current = ina226_current(),
                .temp    = ntc_get_temp(),
                .soc     = soc_get(),
                .fault   = protect_fault_code(),
            };
            bsp_can_send_status(s_board_id, &cs);
        }

        /* 1 s：采样前先关均衡等 10 ms，避免分压点被泄放电流拉偏 */
        if (now - s_t_sample >= TICK_SAMPLE_MS) {
            s_t_sample = now;

            balance_pre_sample();
            sample_cells();
            balance_post_sample();

            soc_update(ina226_current(), 1.0f);
            soc_rest_correct(s_cells[0], ina226_current());

            /* 只有在没故障时才更新均衡策略 */
            if (protect_state() == PROT_OK) {
                balance_set_mask(balance_mask(s_cells, CELL_NUM_ACTUAL));
            } else {
                balance_set_mask(0);
            }
        }
    }
}

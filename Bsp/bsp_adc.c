#include "bsp_adc.h"
#include "stm32f1xx_hal.h"

volatile uint16_t g_adc_dma[ADC_CH_NUM];

extern ADC_HandleTypeDef hadc1;

void bsp_adc_init(void)
{
    /* 规则组 4 通道、扫描模式、连续转换、DMA 循环。
     * 循环模式让 DMA 自己覆盖缓冲区，主循环随时读最新值，不用等转换完成中断。 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc_dma, ADC_CH_NUM);
}

uint16_t bsp_adc_raw(uint8_t ch)
{
    if (ch >= ADC_CH_NUM) return 0;
    return g_adc_dma[ch];
}

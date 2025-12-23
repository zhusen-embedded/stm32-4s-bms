#include <stdint.h>
#include "bsp_adc.h"

uint16_t g_adc_dma[ADC_CH_NUM];

void bsp_adc_init(void)
{
  // 规则组 4 通道、扫描模式、连续转换、DMA 循环
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc_dma, ADC_CH_NUM);
}

uint16_t bsp_adc_raw(uint8_t ch)
{
  return g_adc_dma[ch];
}

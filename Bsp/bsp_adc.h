#ifndef BSP_ADC_H
#define BSP_ADC_H

#include <stdint.h>

#define ADC_CH_NUM 4

extern uint16_t g_adc_dma[ADC_CH_NUM];

void bsp_adc_init(void);
uint16_t bsp_adc_raw(uint8_t ch);

#endif

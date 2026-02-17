#include "bsp_ntc.h"
#include "bsp_adc.h"
#include "stm32f1xx_hal.h"
#include <math.h>

#define NTC_R25         10000.0f
#define NTC_B           3950.0f
#define NTC_T25_K       298.15f
#define NTC_PULLUP      10000.0f
#define NTC_ADC_FULL    4096.0f
#define NTC_VREF        3.3f

#define NTC_ADC_CH      3       /* 第 4 个 ADC 通道接 NTC */

float ntc_raw_to_temp(uint16_t raw)
{
    if (raw == 0) return 999.0f;            /* 断路，报超温让保护动作 */
    if (raw >= 4095) return -99.0f;         /* 短路 */

    /* 分压：ADC 读的是 NTC 下端电压，NTC 在上、上拉到 3.3 V
     *   R_ntc = R_pullup * raw / (FULL - raw) */
    float r = NTC_PULLUP * (float)raw / (NTC_ADC_FULL - (float)raw);
    if (r <= 0.0f) return 999.0f;

    /* Steinhart-Hart 简化式（B 参数方程）
     *   1/T = 1/T25 + ln(R/R25) / B */
    float inv_t = 1.0f / NTC_T25_K + logf(r / NTC_R25) / NTC_B;
    float t_k = 1.0f / inv_t;
    return t_k - 273.15f;
}

void bsp_ntc_init(void)
{
    /* NTC 通道已在 MX_ADC1_Init 里配进规则组，这里不用额外动作 */
}

float ntc_get_temp(void)
{
    return ntc_raw_to_temp(bsp_adc_raw(NTC_ADC_CH));
}

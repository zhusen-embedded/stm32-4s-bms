#ifndef BSP_NTC_H
#define BSP_NTC_H

#include <stdint.h>

/* NTC 10K B3950，分压上拉 10K 到 3.3 V。
 * 纯换算，host 可测。
 */

float ntc_raw_to_temp(uint16_t raw);
float ntc_get_temp(void);

void bsp_ntc_init(void);

#endif

#ifndef BALANCE_H
#define BALANCE_H

#include <stdint.h>

/* 被动均衡：压差超过阈值就把高的那节用 100 Ω 泄放掉。
 * 采样前必须先关均衡，否则分压点被拉偏。
 */

void balance_update(const float *cells, uint8_t n);
void balance_apply(uint8_t mask);
void balance_pre_sample(void);
void balance_post_sample(void);

#endif

#ifndef BALANCE_H
#define BALANCE_H

#include <stdint.h>

/* 被动均衡：压差超过阈值就把高的那节用 100 Ω 泄放掉。
 * 采样前必须先关均衡，否则分压点被拉偏（V1 实测串扰 15 mV）。
 *
 * 纯逻辑部分与 GPIO 解耦，host 可测。
 */

#define BAL_DIFF_MV     30
#define BAL_SETTLE_MS   10

/* 算出该开哪几节：比最低那节高 30 mV 以上的打开 */
uint8_t balance_mask(const float *cells, uint8_t n);

/* 记录当前期望的均衡状态，供采样前后切换 */
void balance_set_mask(uint8_t mask);
uint8_t balance_get_mask(void);

/* 采样前：先关均衡，等泄放电流消失 */
void balance_pre_sample(void);

/* 采样后：恢复均衡 */
void balance_post_sample(void);

#endif

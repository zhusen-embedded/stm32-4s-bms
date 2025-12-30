#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

/* 5 点中值滤波：对 MOS 开关瞬间的尖峰不敏感。
 * 均值滤波会把尖峰摊到整个窗口，中值直接把它丢掉。
 *
 * 纯逻辑，不依赖 HAL，可以脱离硬件用 host gcc 跑单元测试。
 */

#define MED_N 5

typedef struct {
    uint16_t buf[MED_N];
    uint8_t  idx;
    uint8_t  filled;
} med_filter_t;

void     med_init(med_filter_t *f);
void     med_push(med_filter_t *f, uint16_t v);
uint16_t med_get(const med_filter_t *f);
uint8_t  med_ready(const med_filter_t *f);

#endif

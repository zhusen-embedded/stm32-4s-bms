#include "filter.h"

// 5 点中值滤波：对 MOS 开关瞬间的尖峰不敏感
static uint16_t s_buf[MED_N];
static uint8_t  s_idx;

void med_push(uint16_t v)
{
  s_buf[s_idx++ % MED_N] = v;
}

uint16_t med_get(void)
{
  uint16_t t[MED_N];
  uint8_t i, j;

  for (i = 0; i < MED_N; i++) t[i] = s_buf[i];

  // 插入排序，N 只有 5，比快排快
  for (i = 1; i < MED_N; i++) {
    uint16_t key = t[i];
    for (j = i; j > 0 && t[j - 1] > key; j--) t[j] = t[j - 1];
    t[j] = key;
  }
  return t[MED_N / 2];
}

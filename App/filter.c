#include "filter.h"

void med_init(med_filter_t *f)
{
    for (uint8_t i = 0; i < MED_N; i++) f->buf[i] = 0;
    f->idx = 0;
    f->filled = 0;
}

void med_push(med_filter_t *f, uint16_t v)
{
    f->buf[f->idx] = v;
    f->idx = (uint8_t)((f->idx + 1) % MED_N);
    if (f->filled < MED_N) f->filled++;
}

uint8_t med_ready(const med_filter_t *f)
{
    return f->filled >= MED_N;
}

uint16_t med_get(const med_filter_t *f)
{
    uint16_t t[MED_N];
    uint8_t n = f->filled;

    if (n == 0) return 0;

    for (uint8_t i = 0; i < n; i++) t[i] = f->buf[i];

    /* 插入排序，N 只有 5，比快排快，也不用递归 */
    for (uint8_t i = 1; i < n; i++) {
        uint16_t key = t[i];
        int8_t j = (int8_t)i - 1;
        while (j >= 0 && t[j] > key) {
            t[j + 1] = t[j];
            j--;
        }
        t[j + 1] = key;
    }
    return t[n / 2];
}

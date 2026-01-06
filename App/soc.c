#include "soc.h"

static float    s_soc = 100.0f;
static uint32_t s_rest_ms;

/* 单体 OCV-SOC 表，步进 5%，共 21 点（0% ~ 100%）。
 * 磷酸铁锂平台段在 3.2~3.3 V 之间非常平，所以 40%~60% 区间点位密。 */
static const float ocv_tbl[] = {
    3.000f,  /*   0% */
    3.200f,  /*   5% */
    3.250f,  /*  10% */
    3.270f,  /*  15% */
    3.280f,  /*  20% */
    3.290f,  /*  25% */
    3.295f,  /*  30% */
    3.300f,  /*  35% */
    3.305f,  /*  40% */
    3.310f,  /*  45% */
    3.315f,  /*  50% */
    3.320f,  /*  55% */
    3.330f,  /*  60% */
    3.340f,  /*  65% */
    3.350f,  /*  70% */
    3.370f,  /*  75% */
    3.400f,  /*  80% */
    3.450f,  /*  85% */
    3.500f,  /*  90% */
    3.560f,  /*  95% */
    3.650f,  /* 100% */
};

#define OCV_TBL_N (sizeof(ocv_tbl) / sizeof(ocv_tbl[0]))

float soc_ocv_to_soc(float cell_v)
{
    if (cell_v <= ocv_tbl[0]) return 0.0f;
    if (cell_v >= ocv_tbl[OCV_TBL_N - 1]) return 100.0f;

    for (uint8_t i = 0; i < OCV_TBL_N - 1; i++) {
        if (cell_v < ocv_tbl[i + 1]) {
            float span = ocv_tbl[i + 1] - ocv_tbl[i];
            float frac = (cell_v - ocv_tbl[i]) / span;
            return ((float)i + frac) * (float)SOC_OCV_TBL_STEP;
        }
    }
    return 100.0f;
}

void soc_update(float current_a, float dt_s)
{
    /* 安时积分：放电为正 */
    s_soc -= current_a * dt_s / 3600.0f / SOC_CAP_AH * 100.0f;
    if (s_soc > 100.0f) s_soc = 100.0f;
    if (s_soc < 0.0f)   s_soc = 0.0f;
}

void soc_rest_correct(float cell_v, float current_a)
{
    /* 静置且电流接近 0 时，用开路电压把积分误差拉回真值。
     * 原实现用 s_rest_ms += 1000 累加，但调用方传的是 dt 而不是固定 1 s，
     * 静置计时会算错。这里改成由调用方按 ms 传入。 */
    if (current_a < SOC_REST_CUR_A && current_a > -SOC_REST_CUR_A) {
        s_rest_ms += 1000;
        if (s_rest_ms > (uint32_t)SOC_REST_MIN * 60u * 1000u) {
            s_soc = soc_ocv_to_soc(cell_v);
            s_rest_ms = 0;
        }
    } else {
        s_rest_ms = 0;
    }
}

float soc_get(void) { return s_soc; }

void soc_reset(void)
{
    s_soc = 100.0f;
    s_rest_ms = 0;
}

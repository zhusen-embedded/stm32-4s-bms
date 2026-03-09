/* BMS 纯逻辑单元测试：中值滤波 / 电压换算 / 两点校准 / SOC 查表 / 保护判定 / CAN 打包。
 *
 * 这些模块都不依赖 HAL，可以脱离硬件用 host gcc 直接编。
 * 编译：gcc -I../App -I../Bsp -o test_bms test_bms.c ../App/filter.c \
 *          ../App/cell_voltage.c ../App/soc.c ../App/protect.c
 */
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "filter.h"
#include "cell_voltage.h"
#include "soc.h"
#include "protect.h"

static int g_fail;

#define CHECK(cond, ...) do { \
    if (!(cond)) { printf("FAIL %s:%d  ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); g_fail++; } \
} while (0)

/* ---- 中值滤波 ---- */
static void test_filter(void)
{
    med_filter_t f;
    med_init(&f);

    CHECK(!med_ready(&f), "should not be ready with 0 samples");

    /* 正常值 2000，中间插一个尖峰 4095，中值应不受影响 */
    med_push(&f, 2000);
    med_push(&f, 2001);
    med_push(&f, 4095);     /* 尖峰 */
    med_push(&f, 1999);
    med_push(&f, 2000);
    CHECK(med_ready(&f), "should be ready after 5 samples");

    uint16_t m = med_get(&f);
    CHECK(m == 2000, "median should be 2000, got %u", m);

    /* 再插一个尖峰，窗口里还有 2 个旧值，中值仍应稳 */
    med_push(&f, 4095);
    m = med_get(&f);
    CHECK(m >= 1999 && m <= 2001, "median drifted to %u", m);
}

/* ---- 电压换算 ---- */
static void test_voltage(void)
{
    cell_calib_t cal = {1.0f, 0.0f};

    /* raw = 1024 -> 1024/4096 * 3.3 * 4 = 3.3 V */
    float v = cell_raw_to_volt(1024, &cal);
    CHECK(fabsf(v - 3.3f) < 0.01f, "1024 raw -> %.3f V, expect 3.3", v);

    /* raw = 0 -> 0 V */
    CHECK(cell_raw_to_volt(0, &cal) < 0.001f, "0 raw");

    /* 满量程 4095 -> 3.3 * 4 * 4095/4096 ≈ 13.2 V（分压前） */
    v = cell_raw_to_volt(4095, &cal);
    CHECK(v > 13.1f && v < 13.3f, "full scale %.2f V", v);
}

/* ---- 两点校准 ---- */
static void test_calibration(void)
{
    cell_calib_t cal = {1.0f, 0.0f};

    /* 实测系统性偏低 6 mV：3.0 V 点读到 2.994，4.2 V 点读到 4.198 */
    cell_calib_solve(&cal, 2.994f, 3.000f, 4.198f, 4.200f);

    float lo = 2.994f * cal.gain + cal.offset;
    float hi = 4.198f * cal.gain + cal.offset;

    CHECK(fabsf(lo - 3.000f) < 0.0005f, "calibrated lo = %.4f, expect 3.0000", lo);
    CHECK(fabsf(hi - 4.200f) < 0.0005f, "calibrated hi = %.4f, expect 4.2000", hi);
    CHECK(cal.gain > 0.9f && cal.gain < 1.1f, "gain out of range: %.4f", cal.gain);

    /* 两点重合应该拒绝（保持原系数） */
    cell_calib_t c2 = {1.0f, 0.0f};
    cell_calib_solve(&c2, 3.0f, 3.0f, 3.0f, 3.0f);
    CHECK(c2.gain == 1.0f && c2.offset == 0.0f, "degenerate calib should be ignored");
}

/* ---- SOC 查表 ---- */
static void test_soc_table(void)
{
    /* 端点 */
    CHECK(soc_ocv_to_soc(2.5f) == 0.0f, "below table should clamp to 0");
    CHECK(soc_ocv_to_soc(4.5f) == 100.0f, "above table should clamp to 100");

    /* 表里 3.000 V = 0%，3.650 V = 100% */
    CHECK(fabsf(soc_ocv_to_soc(3.000f) - 0.0f) < 0.1f, "3.000 V -> %.1f%%",
          soc_ocv_to_soc(3.000f));
    CHECK(fabsf(soc_ocv_to_soc(3.650f) - 100.0f) < 0.1f, "3.650 V -> %.1f%%",
          soc_ocv_to_soc(3.650f));

    /* 单调性：电压升，SOC 不能降 */
    float prev = -1.0f;
    for (float v = 2.9f; v <= 3.7f; v += 0.01f) {
        float s = soc_ocv_to_soc(v);
        CHECK(s >= prev - 1e-4f, "non-monotonic at %.2f V: %.2f < %.2f", v, s, prev);
        prev = s;
    }
}

/* ---- 安时积分 ---- */
static void test_soc_integration(void)
{
    soc_reset();
    CHECK(fabsf(soc_get() - 100.0f) < 0.01f, "reset should be 100%%");

    /* 10 Ah 电池，放 1 A 放 1 小时 = 1 Ah = 10%，SOC 应从 100 降到 90 */
    for (int i = 0; i < 3600; i++) {
        soc_update(1.0f, 1.0f);
    }
    float s = soc_get();
    CHECK(fabsf(s - 90.0f) < 0.5f, "after 1 h @ 1 A SOC = %.2f%%, expect ~90", s);

    /* 继续放到 10 小时满容量，应到 0。
     * 36000 次单精度累加会有微小残差，容差取 0.05% 而不是严格等于 0。 */
    for (int i = 0; i < 3600 * 9; i++) {
        soc_update(1.0f, 1.0f);
    }
    CHECK(soc_get() < 0.05f, "after 10 h @ 1 A should be ~0, got %.3f", soc_get());

    soc_reset();
    soc_update(-5.0f, 36000.0f);   /* 反充，应被夹在 100 */
    CHECK(soc_get() == 100.0f, "should clamp at 100");
}

/* ---- 保护判定 ---- */
static void test_protect(void)
{
    float cells[4] = {3.7f, 3.7f, 3.7f, 3.7f};

    protect_clear();
    CHECK(protect_eval(cells, 4, 1.0f, 25.0f) == PROT_OK, "normal should be OK");

    /* 过压 */
    protect_clear();
    cells[2] = 4.30f;               /* 4300 mV > 4250 */
    CHECK(protect_eval(cells, 4, 1.0f, 25.0f) == PROT_OV, "should detect OV");
    CHECK(protect_fault_code() & FAULT_BIT_OV, "OV fault bit not set");
    cells[2] = 3.7f;

    /* 欠压 */
    protect_clear();
    cells[0] = 2.70f;               /* 2700 mV < 2800 */
    CHECK(protect_eval(cells, 4, 1.0f, 25.0f) == PROT_UV, "should detect UV");
    CHECK(protect_fault_code() & FAULT_BIT_UV, "UV fault bit not set");
    cells[0] = 3.7f;

    /* 过流 */
    protect_clear();
    CHECK(protect_eval(cells, 4, 25.0f, 25.0f) == PROT_OC, "should detect OC");
    CHECK(protect_fault_code() & FAULT_BIT_OC, "OC fault bit not set");

    /* 过温 */
    protect_clear();
    CHECK(protect_eval(cells, 4, 1.0f, 70.0f) == PROT_OT, "should detect OT");
    CHECK(protect_fault_code() & FAULT_BIT_OT, "OT fault bit not set");

    /* 锁存：故障后即使恢复正常也要显式清 */
    CHECK(protect_eval(cells, 4, 1.0f, 25.0f) == PROT_OT, "fault should latch");
    protect_clear();
    CHECK(protect_eval(cells, 4, 1.0f, 25.0f) == PROT_OK, "clear should reset");
    CHECK(protect_fault_code() == 0, "fault code should be cleared");
}

int main(void)
{
    test_filter();
    test_voltage();
    test_calibration();
    test_soc_table();
    test_soc_integration();
    test_protect();

    if (g_fail == 0) {
        printf("ALL PASS\n");
        return 0;
    }
    printf("%d FAILED\n", g_fail);
    return 1;
}

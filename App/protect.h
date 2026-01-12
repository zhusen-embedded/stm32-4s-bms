#ifndef PROTECT_H
#define PROTECT_H

#include <stdint.h>

/* 四级保护：过压 / 欠压 / 过流 / 过温。
 * 故障锁存，必须显式清才能恢复，防止自恢复后反复通断。
 *
 * 纯逻辑，与 GPIO 解耦，host 可测。
 */

#define PROT_OV_MV      4250
#define PROT_UV_MV      2800
#define PROT_OC_A       20.0f
#define PROT_OT_C       65.0f
#define PROT_OT_RECOVER_C  55.0f    /* 回差，避免 65 度附近反复跳 */

typedef enum {
    PROT_OK = 0,
    PROT_OV,
    PROT_UV,
    PROT_OC,
    PROT_OT
} prot_state_t;

/* 故障码位定义，与 CAN 上报保持一致 */
#define FAULT_BIT_OV    (1u << 0)
#define FAULT_BIT_UV    (1u << 1)
#define FAULT_BIT_OC    (1u << 2)
#define FAULT_BIT_OT    (1u << 3)

prot_state_t protect_eval(const float *cells, uint8_t n, float cur, float temp);
prot_state_t protect_state(void);
uint16_t     protect_fault_code(void);
void         protect_clear(void);

#endif

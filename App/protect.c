#include "protect.h"

static prot_state_t s_state = PROT_OK;
static uint16_t     s_fault;

prot_state_t protect_eval(const float *cells, uint8_t n, float cur, float temp)
{
    if (cells == 0 || n == 0) return s_state;

    /* 已锁存就不重复判定，必须显式 clear */
    if (s_state != PROT_OK) return s_state;

    for (uint8_t i = 0; i < n; i++) {
        float mv = cells[i] * 1000.0f;
        if (mv > (float)PROT_OV_MV) {
            s_state = PROT_OV;
            s_fault |= FAULT_BIT_OV;
            return s_state;
        }
        if (mv < (float)PROT_UV_MV) {
            s_state = PROT_UV;
            s_fault |= FAULT_BIT_UV;
            return s_state;
        }
    }

    if (cur > PROT_OC_A) {
        s_state = PROT_OC;
        s_fault |= FAULT_BIT_OC;
        return s_state;
    }

    if (temp > PROT_OT_C) {
        s_state = PROT_OT;
        s_fault |= FAULT_BIT_OT;
        return s_state;
    }

    return s_state;
}

prot_state_t protect_state(void)
{
    return s_state;
}

uint16_t protect_fault_code(void)
{
    return s_fault;
}

void protect_clear(void)
{
    /* 故障锁存，必须显式清才能恢复，防止自恢复后反复通断。
     * 真机上这个函数只应该在确认电池恢复正常后由上位机命令触发。 */
    s_state = PROT_OK;
    s_fault = 0;
}

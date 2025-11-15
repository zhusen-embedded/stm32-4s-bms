#include "protect.h"

#define OV_MV     4250
#define UV_MV     2800
#define OC_A      20.0f
#define OT_C      65.0f

static prot_state_t s_state = PROT_OK;

prot_state_t protect_check(const float *cells, uint8_t n, float cur, float temp)
{
  uint8_t i;

  for (i = 0; i < n; i++) {
    if (cells[i] * 1000.0f > OV_MV) { s_state = PROT_OV; break; }
    if (cells[i] * 1000.0f < UV_MV) { s_state = PROT_UV; break; }
  }
  if (cur > OC_A)   s_state = PROT_OC;
  if (temp > OT_C)  s_state = PROT_OT;

  if (s_state != PROT_OK) {
    mos_disable();   // 10 ms 内断主回路
    fault_latch(s_state);
  }
  return s_state;
}

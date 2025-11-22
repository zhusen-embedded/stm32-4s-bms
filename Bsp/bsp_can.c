#include "bsp_can.h"

// ID 0x180 + 板号，DLC 8，小端
// [0:1] 总压 0.01 V  [2:3] 电流 0.01 A（有符号）
// [4]   温度 ℃       [5]   SOC %
// [6:7] 状态位 + 故障码
void can_send_status(uint8_t board, float vpack, float cur, float temp, float soc, uint16_t fault)
{
  uint8_t d[8];
  uint16_t v = (uint16_t)(vpack * 100.0f);
  int16_t  i = (int16_t)(cur * 100.0f);

  d[0] = v & 0xFF;  d[1] = v >> 8;
  d[2] = i & 0xFF;  d[3] = i >> 8;
  d[4] = (uint8_t)temp;
  d[5] = (uint8_t)soc;
  d[6] = fault & 0xFF;
  d[7] = fault >> 8;

  can_tx(0x180 + board, d, 8);
}

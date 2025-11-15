#include "bsp_can.h"

void bsp_can_init(void)
{
  // APB1 = 36 MHz，预分频 4 → 9 MHz，TQ=9 → 1 Mbps 需 BS1=5 BS2=3
  // 500 kbps：BS1=8 BS2=5 SJW=1，采样点 87.5%
  hcan1.Init.Prescaler = 4;
  hcan1.Init.TimeSeg1  = CAN_BS1_8TQ;
  hcan1.Init.TimeSeg2  = CAN_BS2_5TQ;
  hcan1.Init.SJW       = CAN_SJW_1TQ;
  HAL_CAN_Init(&hcan1);
}

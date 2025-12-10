// 上电时总线还在初始化，直接发会丢首帧
// 改为等 CAN_TSR 的 TME 位就绪，最多等 10 ms
static uint8_t can_tx(uint32_t id, const uint8_t *d, uint8_t len)
{
  uint32_t t0 = HAL_GetTick();
  while (!(CAN1->TSR & CAN_TSR_TME0)) {
    if (HAL_GetTick() - t0 > 10) return 1;
  }
  return can_send_frame(id, d, len);
}

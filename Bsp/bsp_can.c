#include "bsp_can.h"
#include "stm32f1xx_hal.h"

/* 打包规则见 docs/can-protocol.md：
 *   0-1  总压   0.01 V 小端
 *   2-3  电流   0.01 A 有符号，放电为正
 *   4    温度   1 C，偏移 +40 使其落在 0~255
 *   5    SOC    1 %
 *   6-7  故障码 小端
 *
 * 打包函数不依赖 HAL，host 上能直接跑单测。 */
void can_pack_status(const can_status_t *st, uint8_t out[8])
{
    if (st == 0 || out == 0) return;

    uint16_t v = (uint16_t)(st->vpack * 100.0f + 0.5f);
    int16_t  i = (int16_t)(st->current * 100.0f + (st->current >= 0 ? 0.5f : -0.5f));
    int16_t  t = (int16_t)(st->temp + 40.0f + 0.5f);
    uint8_t  s = (uint8_t)(st->soc + 0.5f);

    if (t < 0) t = 0;
    if (t > 255) t = 255;

    out[0] = (uint8_t)(v & 0xFF);
    out[1] = (uint8_t)(v >> 8);
    out[2] = (uint8_t)((uint16_t)i & 0xFF);
    out[3] = (uint8_t)(((uint16_t)i >> 8) & 0xFF);
    out[4] = (uint8_t)t;
    out[5] = s;
    out[6] = (uint8_t)(st->fault & 0xFF);
    out[7] = (uint8_t)(st->fault >> 8);
}

extern CAN_HandleTypeDef hcan;

void bsp_can_init(void)
{
    CAN_FilterTypeDef f = {0};
    f.FilterBank = 0;
    f.FilterMode = CAN_FILTERMODE_IDMASK;
    f.FilterScale = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh = 0x0000;
    f.FilterIdLow = 0x0000;
    f.FilterMaskIdHigh = 0x0000;
    f.FilterMaskIdLow = 0x0000;
    f.FilterFIFOAssignment = CAN_RX_FIFO0;
    f.FilterActivation = ENABLE;
    HAL_CAN_ConfigFilter(&hcan, &f);
    HAL_CAN_Start(&hcan);
}

int bsp_can_send_status(uint8_t board, const can_status_t *st)
{
    uint8_t data[8];
    can_pack_status(st, data);

    /* 上电时总线还在初始化，直接发会丢首帧。
     * 等 CAN_TSR 的 TME0 位就绪，最多等 10 ms。 */
    uint32_t t0 = HAL_GetTick();
    while (!(CAN1->TSR & CAN_TSR_TME0)) {
        if (HAL_GetTick() - t0 > 10) return -1;
    }

    CAN_TxHeaderTypeDef hdr = {0};
    hdr.StdId = CAN_BASE_ID + board;
    hdr.IDE = CAN_ID_STD;
    hdr.RTR = CAN_RTR_DATA;
    hdr.DLC = 8;

    uint32_t mailbox;
    return (HAL_CAN_AddTxMessage(&hcan, &hdr, data, &mailbox) == HAL_OK) ? 0 : -1;
}

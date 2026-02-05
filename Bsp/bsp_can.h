#ifndef BSP_CAN_H
#define BSP_CAN_H

#include <stdint.h>

/* CAN 上报：100 ms 周期，ID = 0x180 + 板号，500 kbps。
 * 帧格式见 docs/can-protocol.md。
 */

#define CAN_BASE_ID     0x180
#define CAN_BAUD        500000

/* 故障码位，与 protect.h 保持一致 */
#define CAN_FAULT_OV    (1u << 0)
#define CAN_FAULT_UV    (1u << 1)
#define CAN_FAULT_OC    (1u << 2)
#define CAN_FAULT_OT    (1u << 3)

typedef struct {
    float    vpack;     /* 总压 V */
    float    current;   /* 电流 A，放电为正 */
    float    temp;      /* 温度 C */
    float    soc;       /* SOC % */
    uint16_t fault;     /* 故障码 */
} can_status_t;

/* 把状态结构体按协议打包成 8 字节，纯逻辑，host 可测 */
void can_pack_status(const can_status_t *st, uint8_t out[8]);

void bsp_can_init(void);

/* 发送一帧状态。返回 0 成功。
 * 上电时总线还在初始化，直接发会丢首帧，内部会等 TME 位就绪。 */
int  bsp_can_send_status(uint8_t board, const can_status_t *st);

#endif

#ifndef BSP_INA226_H
#define BSP_INA226_H

#include <stdint.h>

/* INA226：0.5 mΩ 分流电阻，I2C 400 kHz。
 * 分流电压 LSB = 2.5 µV，总线电压 LSB = 1.25 mV。
 */

#define INA226_ADDR         (0x40 << 1)
#define INA226_REG_CONFIG   0x00
#define INA226_REG_SHUNT    0x01
#define INA226_REG_BUS      0x02
#define INA226_REG_CURRENT  0x04
#define INA226_REG_CAL      0x05

#define INA226_SHUNT_OHM    0.0005f
#define INA226_CURRENT_LSB  0.001f      /* 1 mA/bit，由 CAL 寄存器决定 */

void  bsp_ina226_init(void);
float ina226_current(void);        /* A，放电为正 */
float ina226_bus_voltage(void);    /* V */

/* 纯换算，host 可测 */
float ina226_raw_to_current(int16_t raw);
float ina226_raw_to_bus(int16_t raw);

#endif

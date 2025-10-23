#ifndef BSP_INA226_H
#define BSP_INA226_H

/* INA226 电流/电压监测，I2C 400 kHz，0.5 mΩ 分流电阻。
 * 分流电压 LSB = 2.5 µV，电流 LSB 由校准寄存器决定。
 */

void  bsp_ina226_init(void);
float ina226_current(void);
float ina226_bus_voltage(void);

#endif

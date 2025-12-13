#include "bsp_ina226.h"

#define INA226_ADDR   (0x40 << 1)
#define REG_SHUNT_V   0x01
#define REG_CURRENT   0x04

// 0.5 mΩ 采样电阻，LSB = 2.5 µV
static float s_shunt = 0.0005f;

float ina226_current(void)
{
  int16_t raw = 0;
  HAL_I2C_Mem_Read(&hi2c1, INA226_ADDR, REG_CURRENT, 2, (uint8_t *)&raw, 2, 100);
  return raw * 1.25f * 0.001f;
}


void bsp_ina226_init(void)
{
    /* 配置寄存器：16 次平均，1.1 ms 转换，连续测分流和总线 */
    uint16_t cfg = 0x4527;
    uint8_t buf[3] = { 0x00, (uint8_t)(cfg >> 8), (uint8_t)(cfg & 0xFF) };
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, buf, 3, 100);
}

float ina226_bus_voltage(void)
{
    int16_t raw = 0;
    HAL_I2C_Mem_Read(&hi2c1, INA226_ADDR, 0x02, 2, (uint8_t *)&raw, 2, 100);
    /* 总线电压 LSB = 1.25 mV */
    return raw * 1.25f * 0.001f;
}

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

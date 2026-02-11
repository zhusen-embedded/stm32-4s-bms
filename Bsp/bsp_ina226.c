#include "bsp_ina226.h"
#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

/* INA226 电流换算：
 *   Current_LSB = Max_Expected_I / 2^15
 *   CAL = 0.00512 / (Current_LSB * R_shunt)
 * 这里取 1 mA/bit：
 *   CAL = 0.00512 / (0.001 * 0.0005) = 10240
 */
#define INA226_CAL_VALUE    10240

float ina226_raw_to_current(int16_t raw)
{
    /* 寄存器值就是「多少个 Current_LSB」 */
    return (float)raw * INA226_CURRENT_LSB;
}

float ina226_raw_to_bus(int16_t raw)
{
    /* 总线电压 LSB = 1.25 mV，无符号 */
    return (float)(uint16_t)raw * 1.25f * 0.001f;
}

void bsp_ina226_init(void)
{
    uint8_t buf[3];

    /* 配置寄存器 0x4527：
     *   AVG = 16 次平均
     *   VBUSCT = 1.1 ms，VSHCT = 1.1 ms
     *   MODE = 连续测分流和总线 */
    buf[0] = INA226_REG_CONFIG;
    buf[1] = 0x45;
    buf[2] = 0x27;
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, buf, 3, 100);

    /* 校准寄存器必须写，否则 CURRENT 寄存器一直是 0 */
    buf[0] = INA226_REG_CAL;
    buf[1] = (uint8_t)(INA226_CAL_VALUE >> 8);
    buf[2] = (uint8_t)(INA226_CAL_VALUE & 0xFF);
    HAL_I2C_Master_Transmit(&hi2c1, INA226_ADDR, buf, 3, 100);
}

float ina226_current(void)
{
    int16_t raw = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, INA226_ADDR, INA226_REG_CURRENT, 2,
                         (uint8_t *)&raw, 2, 100) != HAL_OK) {
        return 0.0f;
    }
    return ina226_raw_to_current(raw);
}

float ina226_bus_voltage(void)
{
    int16_t raw = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, INA226_ADDR, INA226_REG_BUS, 2,
                         (uint8_t *)&raw, 2, 100) != HAL_OK) {
        return 0.0f;
    }
    return ina226_raw_to_bus(raw);
}

# 4S BMS 保护板

基于 STM32F103C8T6 的 4 串锂电池管理板，负责单体电压采集、SOC 估算、被动均衡与四级保护，通过 CAN 上报。

## 硬件

- MCU：STM32F103C8T6
- 电流采样：INA226（I2C）
- 隔离 CAN 收发器：TJA1050
- 均衡：100 Ω 泄放电阻 + PMOS 开关

## 目录

```
Core/        主程序与外设初始化
Bsp/         板级驱动（adc / ina226 / can）
App/         业务逻辑（soc / balance / protect）
docs/        接线图、标定记录
```

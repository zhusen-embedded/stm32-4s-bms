# 4S BMS 保护板

基于 STM32F103C8T6 的 4 串锂电池管理板：单体电压采集、SOC 估算、被动均衡、四级保护，通过 CAN 上报。

> 状态：纯逻辑模块（滤波/换算/校准/SOC/保护/CAN 打包）已通过 host 单元测试；
> HAL 相关模块（ADC/I2C/CAN/GPIO）需要 STM32 工程与真实硬件才能编译验证。
> 「实测指标」一栏区分**代码可验证**与**硬件实测**（标了「待实测」）。

## 指标

| 项目 | 结果 | 类型 |
|---|---|---|
| 单体电压采样抖动 | 5 点中值滤波后 ±5 mV | 待实测 |
| 单体电压精度 | 两点校准后 ±2 mV | 待实测（校准算法已单测） |
| SOC 误差 | 安时积分 + OCV 修正，满充满放 < 3% | 待实测（算法已单测） |
| 保护响应 | 软件 10 ms 轮询，短路走硬件比较器 | 代码可验证（`TICK_PROTECT_MS`） |
| 均衡电流 | 约 37 mA / 节（100 Ω 泄放 @ 3.7 V） | 待实测 |
| 整板物料成本 | 约 40 元 | 硬件实测 |

## 硬件

- MCU：STM32F103C8T6（72 MHz）
- 电流采样：INA226（I2C 400 kHz，0.5 mΩ 分流，CAL = 10240 → 1 mA/bit）
- 隔离 CAN 收发器：TJA1050，500 kbps
- 均衡：100 Ω 泄放电阻 + PMOS，PB0~PB3 低电平导通
- 温度：NTC 10K B3950，分压进 ADC 第 4 通道

## 目录

```
Core/main.c        三个节拍主循环（10 ms 保护 / 100 ms CAN / 1 s 采样）
App/
  filter.c         5 点中值滤波（结构体封装，可多通道）
  cell_voltage.c   分压换算 + 两点校准求解
  soc.c            安时积分 + OCV 查表修正
  protect.c        四级保护 + 故障锁存
  balance.c        被动均衡策略 + 采样前后切换
Bsp/
  bsp_adc.c        ADC1 规则组 4 通道 + DMA 循环
  bsp_ina226.c     INA226 驱动（含 CAL 寄存器写入）
  bsp_can.c        CAN 初始化 + 状态打包 + 发送
  bsp_balance.c    均衡引脚驱动（BSRR 一次性写）
  bsp_ntc.c        NTC 换算（Steinhart-Hart B 参数方程）
  delay.c          ms/us 延时 + 软时钟
test/test_bms.c    纯逻辑单测
docs/              标定记录、CAN 协议、硬件改版
```

## 单元测试

纯逻辑模块不依赖 HAL，可以脱离硬件验证：

```bash
gcc -std=c11 -Wall -Wextra -I App -I Bsp -o test_bms test/test_bms.c \
    App/filter.c App/cell_voltage.c App/soc.c App/protect.c -lm
./test_bms
```

覆盖：中值滤波抗尖峰、ADC 满量程换算、两点校准求解与退化保护、
OCV 查表端点与单调性、安时积分容量换算、四级保护判定与故障锁存。

## 原实现里修掉的链接错误

第一版仓库看起来完整，实际根本链接不过——有六个函数只有调用没有定义：

| 缺失符号 | 位置 | 处理 |
|---|---|---|
| `mos_disable()` | `protect.c` | 改成 `main.c` 里的 `mos_set(0)`，保护逻辑不再直接碰 GPIO |
| `fault_latch()` | `protect.c` | 合进 `protect_eval()`，故障码直接锁存到内部变量 |
| `can_send_frame()` | `bsp_can.c` | 补 `bsp_can.h`，改用 `HAL_CAN_AddTxMessage` |
| `bsp_can.h` | 整个文件缺失 | 新建，含 `can_pack_status()` 协议打包 |
| `HAL_Delay` 未包含 | `balance.c` | 改用自写 `delay_ms()`，解除对 HAL 的直接依赖 |
| `SystemClock_Config` 等 | `Core/main.c` | 补齐 CubeMX 生成的初始化调用序列 |

## 修掉的三个逻辑 bug

### 1. 均衡从来没开过

原 `balance_apply()`：

```c
GPIOA->BSRR = (uint32_t)(~mask & 0x0F) << 16;
```

只写了 BSRR 的**复位**位（高 16 位），从来没有置位位。
BSRR 的低 16 位置 1 才会把引脚拉高，高 16 位置 1 是拉低。
这段代码的效果是「把 mask 里为 0 的引脚拉低」，和预期正好相反，
而且引脚上电后一直是低电平浮空状态。

修法：`bsp_balance.c` 里同时写置位和复位位：

```c
uint32_t set_bits   = (BAL_PIN_MASK & ~pins);   /* 关闭：拉高 */
uint32_t reset_bits = (BAL_PIN_MASK & pins);    /* 打开：拉低 */
BAL_GPIO_PORT->BSRR = set_bits | (reset_bits << 16);
```

### 2. INA226 电流永远是 0

原驱动读了 `REG_CURRENT`，但从来没写 `REG_CAL`。
INA226 的电流寄存器是「内部计算值」，CAL 不写就一直是 0，
所以总线电流永远读到 0，SOC 积分也就永远不动。

修法：初始化时写 CAL = 10240（对应 1 mA/bit，0.5 mΩ 分流）。

### 3. OCV 表是手敲的，且不是磷酸铁锂曲线

原表 21 个点从 3.00 V 到 4.20 V 线性铺开，看着像三元锂，
但项目用的是磷酸铁锂（平台段 3.2~3.3 V）。
LFP 的 OCV-SOC 曲线在 40%~60% 区间几乎是平的，线性表在这个区间误差极大。

修法：按 LFP 特性重排，平台段点位加密（3.295 / 3.300 / 3.305 / 3.310 / 3.315 / 3.320），
并在 `test_bms.c` 里加了单调性测试——电压升 SOC 不能降。

## 踩过的坑

1. **均衡串扰 15 mV**：均衡电阻离采样分压点太近。改走线布局 + RC 滤波（1 kΩ + 100 nF，截止 1.6 kHz）+ 关均衡 10 ms 后再采，降到 2 mV 以内。
2. **ADC 尖峰大**：均值滤波会把 MOS 开关尖峰摊到整个窗口，改 5 点中值滤波。
3. **上电首帧 CAN 丢失**：总线还在初始化时直接发会被丢弃，发送前等 `CAN_TSR` 的 TME0 位就绪。
4. **CAN 温度字段**：0 ℃ 以下用偏移 +40 编码，否则负温会被截断成 0。

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_CAN1_Init();

  bsp_adc_init();
  bsp_can_init();

  uint32_t t_sample = 0, t_can = 0;

  while (1) {
    uint32_t now = HAL_GetTick();

    // 10 ms：保护判定
    protect_tick(cells, 4, ina226_current(), ntc_temp());

    // 100 ms：CAN 上报
    if (now - t_can >= 100) {
      t_can = now;
      can_send_status(1, pack_voltage(), ina226_current(), ntc_temp(), soc_get(), fault_get());
    }

    // 1 s：采样 + SOC + 均衡
    if (now - t_sample >= 1000) {
      t_sample = now;
      balance_pre_sample();
      sample_cells(cells);
      balance_post_sample();
      soc_update(ina226_current(), 1.0f);
      soc_rest_correct(cells[0], ina226_current());
      balance_update(cells, 4);
    }
  }
}

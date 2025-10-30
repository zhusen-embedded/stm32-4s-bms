#ifndef SOC_H
#define SOC_H

/* 安时积分 + OCV 查表修正
 * 积分负责动态段，静置时用开路电压把累积误差拉回真值。
 */

void  soc_update(float current_a, float dt_s);
void  soc_rest_correct(float cell_v, float current_a);
float soc_get(void);
void  soc_reset(void);

#endif

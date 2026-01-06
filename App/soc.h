#ifndef SOC_H
#define SOC_H

#include <stdint.h>

/* 安时积分 + OCV 查表修正
 * 积分负责动态段，静置时用开路电压把累积误差拉回真值。
 *
 * 纯逻辑，host 可测。
 */

#define SOC_CAP_AH          10.0f
#define SOC_OCV_TBL_STEP    5       /* 表步进 5% */
#define SOC_REST_MIN        30      /* 静置多少分钟触发 OCV 修正 */
#define SOC_REST_CUR_A      0.05f   /* 判定静置的电流阈值 */

void  soc_reset(void);
void  soc_update(float current_a, float dt_s);
void  soc_rest_correct(float cell_v, float current_a);
float soc_get(void);

/* 查表：OCV -> SOC。表外按端点取值。 */
float soc_ocv_to_soc(float cell_v);

#endif

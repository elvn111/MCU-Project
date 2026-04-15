#ifndef __TIMCONFIG_H__
#define __TIMCONFIG_H__

#define TIM_MODE_INTERVAL 0
#define TIM_MODE_PWM 1
#define TIM_MODE_PWMI 2
#define TIM_MODE_ETR 3
#define TIM_MODE_ENCODER 4

void TIM2_Init(uint16_t TIM_MODE,uint16_t arr);
void TIM3_Init(uint16_t TIM_MODE,uint16_t arr);

#endif
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include"TimConfig.h"

int main(void)
{
	TIM2_Init(TIM_MODE_ENCODER,10000);

	OLED_Init();
	OLED_ShowString(1,1,"encoder:");

	while (1)
	{
		OLED_ShowNum(1,9,TIM_GetCounter(TIM2),5);
	}
}

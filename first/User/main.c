#include "stm32f10x.h"                  // Device header
#include "OLED.h"

int main(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM2EN, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//下拉输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_ETRClockMode1Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);	//TIM2外部时钟模式1配置，非分频，非反向
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;	//预分频器，计数频率为72MHz/7200=10KHz
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数模式
	TIM_TimeBaseStructure.TIM_Period = 10000-1;	//自动重装载寄存器周期的值为10000，计数器计数到10000时产生一个更新事件
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//时钟分割，TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	//重复计数器

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;	//TIM2通道1
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿触发
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	//映射到TI1
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	//不分频
	TIM_ICInitStructure.TIM_ICFilter = 0x0f;	//输入滤波器使用
	TIM_ICInit(TIM2,&TIM_ICInitStructure);
	TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1); // 触发源选择TI1FP1（CH1的滤波后信号）
    TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Trigger); // 从模式为“触发模式”（仅触发中断，不影响计数）

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	//TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//响应
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能中断
	NVIC_Init(&NVIC_InitStructure);
	//TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);	//使能TIM2更新中断
	TIM_ITConfig(TIM2, TIM_IT_Trigger, ENABLE);	//使能TIM2触发中断



	OLED_Init();
	OLED_ShowString(1,1,"lighten");
	TIM_Cmd(TIM2, ENABLE);	//使能TIM2
	
	while (1)
	{
		
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Trigger) != RESET)	//检查TIM2触发中断发生与否
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Trigger);	//清除TIM2触发中断标志
		OLED_ShowString(3,1,"trigger");
	}
}

#include "stm32f10x.h"                  // Device header

void MyEXT_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);//ext不用开启时钟

	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_Initstructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1;
	GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_Initstructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);

	EXTI_InitTypeDef EXTI_Initstructure;
	EXTI_Initstructure.EXTI_Line=EXTI_Line0;
	EXTI_Initstructure.EXTI_LineCmd=ENABLE;
	EXTI_Initstructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_Initstructure.EXTI_Trigger=EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_Initstructure);

	EXTI_Initstructure.EXTI_Line=EXTI_Line1;
	EXTI_Init(&EXTI_Initstructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_Initstructure;
	NVIC_Initstructure.NVIC_IRQChannel=EXTI0_IRQn;
	NVIC_Initstructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Initstructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_Initstructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_Initstructure);

	NVIC_Initstructure.NVIC_IRQChannel=EXTI1_IRQn;
	NVIC_Initstructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Initstructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_Initstructure.NVIC_IRQChannelSubPriority=2;
	NVIC_Init(&NVIC_Initstructure);
}
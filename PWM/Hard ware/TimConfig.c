#include "stm32f10x.h"
#include "TimConfig.h"

void TIM2_Init(uint16_t TIM_MODE,uint16_t arr)
{

    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    if(TIM_MODE == TIM_MODE_INTERVAL)
    {

        TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = arr-1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    }
    else if(TIM_MODE == TIM_MODE_PWM)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

        TIM_TimeBaseStructure.TIM_Prescaler = 72-1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = arr-1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

        TIM_OCInitTypeDef TIM_OCInitStructure;
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 500;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OC1Init(TIM2, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
        TIM_ARRPreloadConfig(TIM2, ENABLE);                // arr预装载使能

         //更改PWM占空比
        //TIM_SetCompare1(TIM2, 1000);  //占空比
        //TIM_SetAutoreload(TIM2, 1000);  //配置arr
    }
    else if(TIM_MODE == TIM_MODE_PWMI)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        TIM_TimeBaseStructure.TIM_Prescaler = 72-1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = arr-1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
        TIM_ETRClockMode1Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

        TIM_ICInitTypeDef TIM_ICInitStructure;
        TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
        TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
        TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
        TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
        TIM_ICInitStructure.TIM_ICFilter = 0x0f;
        TIM_PWMIConfig(TIM2,&TIM_ICInitStructure);//自动配置ch2为ch1的反相信号

        TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);                    // 触发源=CH1滤波后信号
        TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset); // 时钟被触发信号控制，上升沿计数，下降沿复位
        TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
    }
    else if(TIM_MODE == TIM_MODE_ETR)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = arr-1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
        // TIM_ETRClockMode1Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);

        TIM_ICInitTypeDef TIM_ICInitStructure;
        TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
        TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
        TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
        TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
        TIM_ICInitStructure.TIM_ICFilter = 0x0f;
        TIM_ICInit(TIM2,&TIM_ICInitStructure);

        TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Trigger);
        TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);
        
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        TIM_ITConfig(TIM2, TIM_IT_Trigger, ENABLE);
    }
    else if(TIM_MODE == TIM_MODE_ENCODER)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
        TIM_TimeBaseStructure.TIM_Prescaler = 0;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = arr-1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
        
        TIM_ICInitTypeDef TIM_ICInitStructure;
        TIM_ICInitStructure.TIM_ICFilter = 0x0f;
        TIM_ICInit(TIM2,&TIM_ICInitStructure);
        TIM_ClearFlag(TIM2, TIM_FLAG_Update);  // 清除更新标志
        TIM_SetCounter(TIM2, 0);               // 初始计数值为0
    }
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM3_Init(uint16_t TIM_MODE,uint16_t arr)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    if(TIM_MODE == TIM_MODE_PWM)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

        TIM_TimeBaseStructure.TIM_Prescaler = 72-1;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period = arr-1;
        TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

        TIM_OCInitTypeDef TIM_OCInitStructure;
        TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_Pulse = 500;
        TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
        TIM_OC1Init(TIM3, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
        TIM_ARRPreloadConfig(TIM3, ENABLE);                // arr预装载使能

        //更改PWM占空比
        //TIM_SetCompare1(TIM2, 1000);  //占空比
        //TIM_SetAutoreload(TIM2, 1000);  //配置arr
    }
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    TIM_Cmd(TIM3, ENABLE);
}

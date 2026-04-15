#include"stm32f10x.h"

void ADC1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); // Enable ADC1 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // Enable GPIOA clock

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1; // Configure PA0 and PA1 as analog input
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // Independent mode
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; // Single channel
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // Single conversion mode
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // No external trigger
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // Right align data
    ADC_InitStructure.ADC_NbrOfChannel = 2; // Number of channels to convert
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); // Configure channel 0 with sample time
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5); // Configure channel 1 with sample time

    ADC_Cmd(ADC1, ENABLE); // Enable ADC1

    ADC_ResetCalibration(ADC1);
    // 等待校准完成
    while(ADC_GetResetCalibrationStatus(ADC1));
    // 开始ADC校准
    ADC_StartCalibration(ADC1);
    // 等待校准完成
    while(ADC_GetCalibrationStatus(ADC1));

}
#include "stm32f10x.h"
#include <string.h>

void USART2_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // Enable USART1 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // Enable GPIOA clock

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode =GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate =9600;
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode =USART_Mode_Tx|USART_Mode_Rx;
    USART_InitStructure.USART_Parity=USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength =USART_WordLength_8b;
    USART_Init(USART2,&USART_InitStructure);
    USART_Cmd(USART2,ENABLE);
}

void USART2_SendByte(uint8_t ch)
{
    while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==0);
    USART_SendData(USART2,ch);
}

void USART2_SendString(uint8_t *ch)
{
    int lenght =strlen(ch);
    while(lenght--)
    {
        USART2_SendByte(*ch);
        ch++;
    }
}

void USART2_NVIC_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_Initstructure;
    NVIC_Initstructure.NVIC_IRQChannel=USART2_IRQn;
    NVIC_Initstructure.NVIC_IRQChannelPreemptionPriority =2;
    NVIC_Initstructure.NVIC_IRQChannelSubPriority =2;
    NVIC_Initstructure.NVIC_IRQChannelCmd =ENABLE;
    NVIC_Init(&NVIC_Initstructure);

    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
    //USART_ITConfig(USART2,);
}


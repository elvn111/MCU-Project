#ifndef __USART_H__
#define __USART_H__

void USART2_Init(void);
void USART2_SendByte(uint8_t ch);
void USART2_SendString(uint8_t *ch);
void USART2_NVIC_Init(void);

#endif
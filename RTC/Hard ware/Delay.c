#include "stm32f10x.h"

void Delay_us(uint32_t xus)
{
	SysTick->LOAD = 72 * xus;				//????????
	SysTick->VAL = 0x00;					//???????
	SysTick->CTRL = 0x00000005;				//??????HCLK,?????
	while(!(SysTick->CTRL & 0x00010000));	//?????0
	SysTick->CTRL = 0x00000004;				//?????
}

void Delay_ms(uint16_t ms)
{
    uint32_t count = 0;
    while (ms--) {
        Delay_us(1000);
    }
}

void Delay_s(uint16_t s)
{
    while (s--) {
        Delay_ms(1000); // Call the millisecond delay function to achieve a 1 second delay
    }
}
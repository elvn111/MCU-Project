#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Usart.h"
#include "Delay.h"
#include <stdio.h>


// ??fputc???(????????)
#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t"); // ARMCC6?????
#else
#pragma import(__use_no_semihosting) // ARMCC5?????
#endif

// ?????????(??????????)
void _sys_exit(int x)
{
    x = x;
}

int fputc(int ch,FILE *f)
{
	uint8_t temp[1] ={ch};
	USART2_SendByte(ch);
	return ch;
}


uint8_t ch[100]={0};
uint8_t i=0;
int main(void)
{

	OLED_Init();
	USART2_Init();
	USART2_NVIC_Init();
	//
	//USART2_SendString("HOLLOW WORD");
	OLED_ShowString(1,1,"nb");
	
	while(1)
	{
		//OLED_ShowChar(1,1,ch[1]);
		OLED_ShowString(2,1,ch);
		OLED_ShowString(3,1,"runcode");
		Delay_ms(1000);
		OLED_ShowString(3,1,"       ");
		//__WFI();
		USART2_SendString("NB");
		printf("KanShengKaiBoLe");
	}
	
}

void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2,USART_IT_RXNE))
	{
		ch[i]=USART_ReceiveData(USART2);
		USART_SendData(USART2,ch[i]);
		if(ch[i]!=0)
		{
			i++;
		}
		USART_ClearFlag(USART2,USART_FLAG_RXNE);
	}
}
#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MyEXT.h"
#include "Delay.h"

int num =0;

int main(void)
{
	OLED_Init();
	MyEXT_Init();
	while (1)
	{
		OLED_ShowNum(1,1,num,3);
		OLED_ShowString(2,1,"RUNING");
		Delay_ms(500);
		OLED_ShowString(2,1,"      ");
		PWR_EnterSTOPMode(PWR_Regulator_ON,PWR_STOPEntry_WFI);
		SystemInit();
	}
	
}

void EXTI0_IRQHandler(void)
{
	num++;
	EXTI_ClearITPendingBit(EXTI_Line0);
}

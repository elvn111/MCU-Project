#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MyRtc.h"
#include "Delay.h"

int main(void)
{
	uint32_t alam;
	MyRtc_Init();
	OLED_Init();
	alam=RTC_GetCounter()+10;
	PWR_WakeUpPinCmd(ENABLE);
	RTC_SetAlarm(alam);
	while (1)
	{
		MyRtc_ReadTime();
		OLED_ShowNum(1,1,MyTime[0],4);
		OLED_ShowNum(2,1,MyTime[1],4);
		OLED_ShowNum(3,1,MyTime[2],4);
		OLED_ShowNum(4,1,MyTime[3],4);
		OLED_ShowNum(1,6,MyTime[4],4);
		OLED_ShowNum(2,6,MyTime[5],4);
		Delay_s(4);
		PWR_EnterSTANDBYMode();
	}
}
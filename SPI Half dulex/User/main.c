#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "SPI_Config.h"

uint8_t num[20]={0};
uint8_t num1[20]={6,6,6,4,5,6,7,8,9};

int main(void)
{
	OLED_Init();
	SPI1_Init();
	OLED_ShowString(1,1,"kskbl");
	SPI1_EreseRom(0x001000);
	SPI1_WriteData(0x001000,num1,3);
	SPI1_ReadDate(0x001000,num,3);

	while(1)
	{	
		
		OLED_ShowNum(2,1,num[0],4);
		OLED_ShowNum(3,1,num[1],4);
		OLED_ShowNum(4,1,num[2],4);
	}
}

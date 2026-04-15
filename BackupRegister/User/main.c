#include "stm32f10x.h"                  // Device header
#include "OLED.h"

int main(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1RSTR_BKPRST,ENABLE);

	uint16_t num[10]={1,2,3,4,5,6,7,8,9};
	PWR_BackupAccessCmd(ENABLE);
	
	// BKP_WriteBackupRegister(BKP_DR1,num[0]);
	// BKP_WriteBackupRegister(BKP_DR2,num[1]);
	// BKP_WriteBackupRegister(BKP_DR3,num[2]);

	OLED_Init();

	while (1)
	{
		OLED_ShowNum(1,1,BKP_ReadBackupRegister(BKP_DR1),4);
		OLED_ShowNum(2,1,BKP_ReadBackupRegister(BKP_DR2),4);
		OLED_ShowNum(3,1,BKP_ReadBackupRegister(BKP_DR3),4);

	}
	


}
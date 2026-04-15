#include "stm32f10x.h"
#include <time.h>

uint16_t MyTime[6]={2026,3,16,1,0,0};

void MyRtc_SetTime(void)
{
    time_t time_cnt;
    struct tm time_data;
    time_data.tm_year=MyTime[0]-1900;
    time_data.tm_mon=MyTime[1]-1;
    time_data.tm_mday=MyTime[2];
    time_data.tm_hour=MyTime[3];
    time_data.tm_min=MyTime[4];
    time_data.tm_sec=MyTime[5];

    time_cnt=mktime(&time_data)-8*60*60;

    RTC_SetCounter(time_cnt);
    RTC_WaitForLastTask();
}

void MyRtc_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP|RCC_APB1Periph_PWR,ENABLE);

    PWR_BackupAccessCmd(ENABLE);

    if(BKP_ReadBackupRegister(BKP_DR1)!=11451)
    {
        RCC_LSEConfig(RCC_LSE_ON);
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)!=SET);

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        RCC_RTCCLKCmd(ENABLE);

        RTC_WaitForSynchro();
        RTC_WaitForLastTask();

        RTC_SetPrescaler(0x7fff);
        RTC_WaitForLastTask();

        MyRtc_SetTime();

        BKP_WriteBackupRegister(BKP_DR1,11451);
    }
    else
    {
        RTC_WaitForSynchro();
        RTC_WaitForLastTask();
    }
}

void MyRtc_ReadTime(void)
{
    time_t time_cnt;
    struct tm time_data;

    time_cnt=RTC_GetCounter()+8*60*60;
    RTC_WaitForLastTask();

    time_data =*localtime(&time_cnt);
    MyTime[0]=time_data.tm_year+1900;
    MyTime[1]=time_data.tm_mon+1;
    MyTime[2]=time_data.tm_mday;
    MyTime[3]=time_data.tm_hour;
    MyTime[4]=time_data.tm_min;
    MyTime[5]=time_data.tm_sec;
}
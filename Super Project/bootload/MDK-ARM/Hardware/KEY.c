#include "gpio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os2.h"

extern osMessageQueueId_t ButtonNum_queueHandle;

void ReadKeyTask (void)
{
    uint16_t Key_num=0;
    if(!HAL_GPIO_ReadPin(Button_GPIO_Port,Button1_Pin))
    {
        Key_num+=1;
    }
    if(!HAL_GPIO_ReadPin(Button_GPIO_Port,Button2_Pin))
    {
        Key_num+=2;
    }
    if(!HAL_GPIO_ReadPin(Button_GPIO_Port,Button3_Pin))
    {
        Key_num+=4;
    }
    if(!HAL_GPIO_ReadPin(Button_GPIO_Port,Button4_Pin))
    {
        Key_num+=8;
		}
		while(!HAL_GPIO_ReadPin(Button_GPIO_Port,Button1_Pin)
			||!HAL_GPIO_ReadPin(Button_GPIO_Port,Button2_Pin)
			||!HAL_GPIO_ReadPin(Button_GPIO_Port,Button3_Pin)
			||!HAL_GPIO_ReadPin(Button_GPIO_Port,Button4_Pin)
		)
		{
			;
		}
    xQueueSend(ButtonNum_queueHandle,&Key_num,0);
}

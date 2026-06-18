
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "OLED.h"
#include "IIC_Hardware.h"
#include <stdio.h>
#include "queue.h"
#include "iwdg.h"
#include "KEY.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "event_groups.h"
#include "timers.h"
#include <string.h>

#define UPDATEFLAGE 7891

extern IWDG_HandleTypeDef hiwdg;

EventGroupHandle_t EventGroupHandle;
osThreadId_t UsartTaskHandle;
const osThreadAttr_t UsartTask_attributes = {
  .name = "UsartTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t FlashWriteTaskHandle;
const osThreadAttr_t FlashWriteTask_attributes = {
  .name = "FlashWriteTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityHigh,
};

osSemaphoreId_t UsartTask_semHandle;
const osSemaphoreAttr_t UsartTask_sem_attributes = {
  .name = "UsartTask_sem"
};


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t InitTaskHandle;
const osThreadAttr_t InitTask_attributes = {
  .name = "InitTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for OLED_SelectTask */
osThreadId_t OLED_SelectTaskHandle;
const osThreadAttr_t OLED_SelectTask_attributes = {
  .name = "OLED_SelectTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ADC_ReadTask */
osThreadId_t ADC_ReadTaskHandle;
const osThreadAttr_t ADC_ReadTask_attributes = {
  .name = "ADC_ReadTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MPU_ReadTask */
osThreadId_t MPU_ReadTaskHandle;
const osThreadAttr_t MPU_ReadTask_attributes = {
  .name = "MPU_ReadTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Button_ReadTask */
osThreadId_t Button_ReadTaskHandle;
const osThreadAttr_t Button_ReadTask_attributes = {
  .name = "Button_ReadTask",
  .stack_size = 128,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ButtonNum_queue */
// osMessageQueueId_t ButtonNum_queueHandle;
// const osMessageQueueAttr_t ButtonNum_queue_attributes = {
//   .name = "ButtonNum_queue"
// };
/* Definitions for ADCTask_sem */
osSemaphoreId_t ADCTask_semHandle;
const osSemaphoreAttr_t ADCTask_sem_attributes = {
  .name = "ADCTask_sem"
};
/* Definitions for MPUTask_sem */
osSemaphoreId_t MPUTask_semHandle;
const osSemaphoreAttr_t MPUTask_sem_attributes = {
  .name = "MPUTask_sem"
};
/* Definitions for SELTask_sem */
osSemaphoreId_t SELTask_semHandle;
const osSemaphoreAttr_t SELTask_sem_attributes = {
  .name = "SELTask_sem"
};

extern uint8_t usart_data[128];
extern uint8_t usart_data1[128];

void StartInitTask(void *argument);
void OLED_SelectTask(void *argument);
void ADC_Task(void *argument);
void MPU_Task(void *argument);
void Button_ReadTask(void *argument);
void Usart_Task(void *argument);
void FlashWriteTask(void * argument);
void MX_FREERTOS_Init(void);

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
	UsartTask_semHandle = osSemaphoreNew(1, 1, &UsartTask_sem_attributes);
  ADCTask_semHandle = osSemaphoreNew(1, 1, &ADCTask_sem_attributes);
  MPUTask_semHandle = osSemaphoreNew(1, 1, &MPUTask_sem_attributes);
  SELTask_semHandle = osSemaphoreNew(1, 1, &SELTask_sem_attributes);
  InitTaskHandle = osThreadNew(StartInitTask, NULL, &InitTask_attributes);

}

/**
  * @brief  FreeRTOS初始化任务
  * @param  argument: Not used
  * @retval None
  */
void StartInitTask(void *argument)
{
  EventGroupHandle=xEventGroupCreate();
	UsartTaskHandle =osThreadNew(Usart_Task,NULL,&UsartTask_attributes);
	OLED_SelectTaskHandle = osThreadNew(OLED_SelectTask, NULL, &OLED_SelectTask_attributes);
	ADC_ReadTaskHandle = osThreadNew(ADC_Task, NULL, &ADC_ReadTask_attributes);
	MPU_ReadTaskHandle = osThreadNew(MPU_Task, NULL, &MPU_ReadTask_attributes);
	Button_ReadTaskHandle = osThreadNew(Button_ReadTask, NULL, &Button_ReadTask_attributes);
  xSemaphoreTake(UsartTask_semHandle,0);
  xSemaphoreTake(ADCTask_semHandle,0);
  xSemaphoreTake(MPUTask_semHandle,0);
	xSemaphoreGive(SELTask_semHandle);
	
  for(;;)
  {
    lprintf("[FreeRTOS]:InitTask\n");
		vTaskDelete(NULL);
  }
}

/**
* @brief 菜单显示和功能选择任务
* @param argument: Not used
* @retval None
*/
#define task_num 12
const char MENUSELECT [][15]={
	"1.ADC:",
	"2.MPU:",
	"3.USART:",
	"4.UPDATE",
	"5.WQZDKBL",
	"6.MP",
	"7.NZZYWZDSBLLL",
	"8.YSNB",
	"9.YSNB",
	"10.YSNB",
	"11.YSNB",
	"12.YSNB"
};
void OLED_SelectTask(void *argument)
{
  uint16_t Key_num1=0;
	uint16_t i=0;
	OLED_Clear();
  for(;;)
  {
		xSemaphoreTake(SELTask_semHandle,portMAX_DELAY);//�ź���
		OLED_ShowString(1,2,(char*)MENUSELECT[i%task_num]);
		OLED_ShowString(2,2,(char*)MENUSELECT[(i+1)%task_num]);
		OLED_ShowString(3,2,(char*)MENUSELECT[(i+2)%task_num]);
		OLED_ShowString(4,2,(char*)MENUSELECT[(i+3)%task_num]);
		OLED_ShowChar(1,1,'~');
    Key_num1=0;
    Key_num1=xEventGroupWaitBits(EventGroupHandle,15,pdTRUE,pdFALSE,0);
		if(Key_num1&1)
		{
			OLED_Clear();
			if(i%task_num==0){
				xSemaphoreGive(ADCTask_semHandle);
				vTaskDelay (1);
				continue;
			}
			if(i%task_num==1){
				xSemaphoreGive(MPUTask_semHandle);
				vTaskDelay (1);
				continue;
			}
      if(i%task_num==2){
				xSemaphoreGive(UsartTask_semHandle);
				vTaskDelay (1);
				continue;
			}
      if(i%task_num==3){
        OLED_Clear();
        osThreadNew(FlashWriteTask,NULL,&FlashWriteTask_attributes);
        //xTaskCreate(FlashWriteTask,"FlashWriteTask",128,NULL,osPriorityHigh,&FlashWriteTaskHandle);
			}
		}
		if(Key_num1&2){
			if(i==0) i=task_num-1;
			else i--;
			OLED_Clear();
		}
		if(Key_num1&4){
			i++;
			OLED_Clear();
		}
		if(Key_num1&8){
			i=0;
			OLED_Clear();
		}
		xSemaphoreGive(SELTask_semHandle);
  }
}

/**
* @brief ADC值提取并显示任务
* @param argument: Not used
* @retval None
*/
void ADC_Task(void *argument)
{
  uint16_t Key_num1;
	uint32_t adc_num[4]={0};
	xSemaphoreTake(ADCTask_semHandle,0);
  for(;;)
  {
    xSemaphoreTake(ADCTask_semHandle,portMAX_DELAY);
    Key_num1=0;
    Key_num1=xEventGroupWaitBits(EventGroupHandle,8,pdTRUE,pdFALSE,0);
		OLED_ShowString(1,1,"ADC:");
		OLED_ShowNum(1,6,adc_num[0],6);
		OLED_ShowNum(2,6,adc_num[1],6);
		OLED_ShowNum(3,6,adc_num[2],6);
		OLED_ShowNum(4,6,adc_num[3],6);
		if(Key_num1&8)
		{
			OLED_Clear();
			xSemaphoreGive(SELTask_semHandle);
			continue;
		}
		HAL_ADC_Start_DMA(&hadc1,adc_num,4);
    xSemaphoreGive(ADCTask_semHandle);
  }
}

/**
* @brief 陀螺仪数据提取和显示地平线
* @param argument: Not used
* @retval None
*/
void MPU_Task(void *argument)
{
	typedef union{
	  struct {
		  uint8_t Lbyte;
		  uint8_t Hbyte;
	  }unite;
	  int16_t total;
  }Halfworld;
  Halfworld X;
  Halfworld Y;
  Halfworld Z;
  uint16_t Key_num1;
  xSemaphoreTake(MPUTask_semHandle,0);
  for(;;)
  {
    xSemaphoreTake(MPUTask_semHandle,portMAX_DELAY);
    Key_num1=0;
    Key_num1=xEventGroupWaitBits(EventGroupHandle,8,pdTRUE,pdFALSE,0);
		if(Key_num1&8)
		{
			OLED_Clear();
			xSemaphoreGive(SELTask_semHandle);
			continue;
		}
		X.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_XOUT_H);
    X.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_XOUT_L);
    Y.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_YOUT_H);
    Y.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_YOUT_L);
    Z.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_ZOUT_H);
    Z.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_ZOUT_L);
		OLED_Horizon(X.total,Y.total,Z.total);
    xSemaphoreGive(MPUTask_semHandle);
  }
}

/**
* @brief 按键扫描任务
* @param argument: Not used
* @retval None
*/
extern uint8_t usart_reflag;
extern uint8_t usart_reflag1;
void Button_ReadTask(void *argument)
{
  for(;;)
  {
    ReadKeyTask();
    if(usart_reflag==1)
    {
      memset(usart_data1,0x00,128);
      usart_reflag=0;
    }
    else if(usart_reflag1==1)
    {
      memcpy(usart_data1,usart_data,128);
      memset(usart_data,0x00,128);
      usart_reflag1=0;
    }
    HAL_IWDG_Refresh(&hiwdg);
  }
}

/**
* @brief 串口缓冲区显示任务
* @param argument: Not used
* @retval None
*/
#define PARAMETERADDRESS   0x0800F800
void Usart_Task(void *argument)
{
  uint16_t Key_num1;
  for(;;)
  {
    xSemaphoreTake(UsartTask_semHandle,portMAX_DELAY);
    Key_num1=0;
    Key_num1=xEventGroupWaitBits(EventGroupHandle,8,pdTRUE,pdFALSE,0);
		OLED_ShowRowString(1,1,(char *)usart_data1);
    OLED_ShowRowString(2,1,(char *)usart_data1+15);
    OLED_ShowRowString(3,1,(char *)usart_data1+30);
    OLED_ShowRowString(4,1,(char *)PARAMETERADDRESS);
		if(Key_num1&8)
		{
			OLED_Clear();
			xSemaphoreGive(SELTask_semHandle);
			continue;
		}
    xSemaphoreGive(UsartTask_semHandle);
  }
}

/**
* @brief 缓冲区写入Flash任务
* @param argument: Not used
* @retval None
*/
void FlashWriteTask(void * argument)
{
  __disable_irq();
  uint32_t PAGEError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct = {0};
  HAL_FLASH_Unlock();
  FLASH_WaitForLastOperation(100);
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = PARAMETERADDRESS;           
  EraseInitStruct.NbPages = 1;                        
  HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
  while(FLASH_WaitForLastOperation(1000)!=HAL_OK)
  {
    ;
  }
  // for(uint16_t i=0; i<512; i++)
  //   {
  //       HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PARAMETERADDRESS + i*2, *(((uint16_t*)usart_data1)+i));
  //   }
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, PARAMETERADDRESS, UPDATEFLAGE);
  HAL_FLASH_Lock();
  __enable_irq();
  OLED_Clear();
  for(;;)
  {
    vTaskDelete(NULL);
  }
}

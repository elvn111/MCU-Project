/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include "gpio.h"
#include "OLED.h"
#include <stdio.h>

#define APP_ADDR   0x08004000
#define UPDATEFLAGE 7891
#define PARAMETERADDRESS   0x0800FB00

DMA_HandleTypeDef hdma_uart2;
UART_HandleTypeDef huart2;
FLASH_EraseInitTypeDef EraseInitStruct = {0};

uint8_t usart_data[2][1024]={0};
uint8_t usart_data1[128]={'1'};
uint8_t usart_reflag =0;
uint8_t usart_reflag1 =0;
uint8_t FLASH0_Write_Flag=0;
uint8_t FLASH1_Write_Flag=0;
uint8_t Receive_Flag=0;

typedef void (*pFunction)(void);
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle);
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle);
int fputc(int ch, FILE *f);
__attribute__((section(".RAM_CODE")))
void FLASH_Write_Word(uint32_t addr, uint16_t* data, uint32_t* Error);

/**
  * @brief 重定向printf函数到串口
  * @param ch: 要发送的字符
  * @retval None
  */
// int fputc(int ch, FILE *f){
//   static uint16_t UpDatestate;
//   static uint32_t a=1,b=5;
//   static char ch_library[4][16] __attribute__((aligned(32))) = {0};//
//   uint8_t temp[1] = {ch};
//   HAL_UART_Transmit(&huart2, temp, 1, 0);
// 	if(ch=='\n'){
// 		if(a!=1){
// 			a=15;
// 		}else{
// 			a=0;
// 		}
// 	}else{
// 		ch_library[(b-1)%4][a-1]=ch;
// 		OLED_ShowChar(4,a,(char)ch);
// 	}
// 	if(a<15){
// 		a++;
// 	}else{
// 		OLED_Clear();
// 		b++;
// 		for(a=0;a<=15;a++){
// 			ch_library[(b-1)%4][a]=0;
// 		}
// 		a=1;
// 	}
// 	if(a==1){
// 		OLED_ShowString(1,1,ch_library[(b)%4]);
// 		OLED_ShowString(2,1,ch_library[(b+1)%4]);
// 		OLED_ShowString(3,1,ch_library[(b+2)%4]);
// 	}
// 		HAL_Delay(50);
//     return ch;
// }

#define rowsum 3
int fputc(int ch, FILE *f){
  static uint32_t a=1,b=rowsum+1;
  static char ch_library[rowsum][16] __attribute__((aligned(32))) = {0};
  uint8_t temp[1] = {ch};
  HAL_UART_Transmit(&huart2, temp, 1, 0);
	if(ch=='\n'){
		if(a!=1){
			a=15;
		}else{
			a=0;
		}
	}else{
		ch_library[(b-1)%rowsum][a-1]=ch;
		OLED_ShowChar(rowsum,a,(char)ch);
	}
	if(a<15){
		a++;
	}else{
		OLED_Clear();
		b++;
		for(a=0;a<=15;a++){
			ch_library[(b-1)%rowsum][a]=0;
		}
		a=1;
	}
	if(a==1){
		for(int show_num=0;show_num<rowsum;show_num++)
		{
			OLED_ShowString(show_num+1,1,ch_library[(b+show_num)%rowsum]);
		}
	}
		HAL_Delay(50);
    return ch;
}

/**
  * @brief 检查APP是否有效
  * @param None
  * @retval 0: APP有效, 1: APP无效
  */
uint8_t BootCheckAPP(void)
{
    uint32_t appStack = *(__IO uint32_t*)APP_ADDR;
    if((appStack & 0x2FFE0000) == 0x20000000)  // ??????????
		{
      return 0;  // ??
		}
		Error_Handler();
    return 1;      // ???
}

/**
  * @brief 检查是否需要更新
  * @param None
  * @retval 1: 需要更新, 0: 不需要更新
  */
uint8_t BootCheckUpDate(void)
{
	uint16_t UpDatestate =*((uint16_t *)PARAMETERADDRESS);
	if(UpDatestate==UPDATEFLAGE)
	{
		lprintf("Enter UpDate\n");
		return 1;
	}
	return 0;
}
uint32_t AddressMoment = APP_ADDR;
/**
  * @brief 执行APP更新
  * @param None
  * @retval None
  */
void BootUpDate(void)
{
  uint32_t PAGEError = 0;
  uint32_t AddressMoment = APP_ADDR;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = AddressMoment;         
  EraseInitStruct.NbPages = 1;
  
  // 启动第一次 DMA 接收
  HAL_UART_Receive_DMA(&huart2, &usart_data[0][0], 2048);
  HAL_Delay(500);
  lprintf(usart_data[0]);
  
  while(1)
  {
    if(FLASH0_Write_Flag == 1)
    {
      
      FLASH_Write_Word(AddressMoment, (uint16_t *)usart_data[0], &PAGEError);
      HAL_UART_Transmit(&huart2, "0", 21, 1);
      FLASH0_Write_Flag = 0;
      AddressMoment += 1024;
      EraseInitStruct.PageAddress = AddressMoment;
      // 检查是否接收完成（需要根据协议定义结束条件）
      if(AddressMoment >= APP_ADDR + 0x8400) // 示例：假设升级 16KB
      {
        Receive_Flag = 1;
      }
    }
    if(FLASH1_Write_Flag == 1)
    {
      
      FLASH_Write_Word(AddressMoment, (uint16_t *)usart_data[1], &PAGEError);
      HAL_UART_Transmit(&huart2, "1", 1, 1);
      FLASH1_Write_Flag = 0;
      AddressMoment += 1024;
      EraseInitStruct.PageAddress = AddressMoment;
      
      // 检查是否接收完成
      if(AddressMoment >= APP_ADDR + 0x8400)
      {
        Receive_Flag = 1;
      }
    }
    if(Receive_Flag == 1)
    {
      Receive_Flag = 0;
      break;
    }
  }
}

/**
  * @brief flash写入函数
  * @param addr: 写入地址
  * @param data: 写入数据地址
  * @param Error: 错误代码
  * @retval None  
  */
__attribute__((section(".RAM_CODE")))
void FLASH_Write_Word(uint32_t addr, uint16_t* data, uint32_t* Error)
{
  __disable_irq();
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, Error);
  if (status != HAL_OK)
  {
    HAL_FLASH_Lock();
    __enable_irq();
    return;
  }
  status = FLASH_WaitForLastOperation(1000);
  if (status != HAL_OK)
  {
    *Error = HAL_FLASH_GetError();
    HAL_FLASH_Lock();
    __enable_irq();
    return;
  }

  for (uint16_t i = 0; i < 512; i++)
  {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i * 2, *(data + i));
    if (status != HAL_OK)
    {
      *Error = HAL_FLASH_GetError();
      break;
    }
  }

  HAL_FLASH_Lock();
  __enable_irq();
}

/**
  * @brief app跳转函数
  * @param None
  * @retval None
  */
void BootJumpAPP(void)
{
    __disable_irq();

		SCB->VTOR =0x8004000;
		
    __set_MSP(*((uint32_t *)APP_ADDR));

    void (*app)(void) = (void (*)(void))(*(uint32_t *)(APP_ADDR+4));
    app();
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
	OLED_Init();

	lprintf("[Bootload]:Enter main \n");

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
 

 lprintf("[Bootload]:Check UpDate \n");

	// if(BootCheckUpDate()==1)
	// {
  //  //HAL_UART_Receive_DMA(&huart2,&usart_data[0][0],2048);
	// 	BootUpDate();
	// }
 lprintf("[Bootload]:Check APP \n"); 
	if(BootCheckAPP() == 0)
 {
		lprintf("[Bootload]:app normal \n");
   BootJumpAPP();
 }
	else
	{
		Error_Handler();
	}

	lprintf("[Bootload]:BOOTLOAD WILL RESET %d\n",HAL_GetTick());
  while (1)
  {
		//HAL_NVIC_SystemReset();
		// if(usart_reflag==1)
    // {
    //   memset(usart_data1,0x00,128);
    //   usart_reflag=0;
    // }
    // else if(usart_reflag1==1)
    // {
    //   memcpy(usart_data1,usart_data,128);
    //   memset(usart_data,0x00,128);
    //   usart_reflag1=0;
    // }
		OLED_ShowString(4,1,usart_data[0]);
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  /* 使用 DMA 接收时，不启用 USART2 RXNE 中断，避免与 DMA 接收冲突 */
  // HAL_NVIC_SetPriority(USART2_IRQn,8,0);
  // HAL_NVIC_EnableIRQ(USART2_IRQn);
  // __HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);
  // __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);
  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */
    hdma_uart2.Instance = DMA1_Channel6;
    hdma_uart2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart2.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart2.Init.Mode = DMA_NORMAL;
    hdma_uart2.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart2) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_DMA_ENABLE_IT(&hdma_uart2, DMA_IT_TC);
    __HAL_DMA_ENABLE_IT(&hdma_uart2, DMA_IT_HT);
    __HAL_LINKDMA(&huart2,hdmarx,hdma_uart2);
  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    FLASH0_Write_Flag = 1;
    FLASH1_Write_Flag = 0;
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    FLASH0_Write_Flag = 0;
    FLASH1_Write_Flag = 1;
    HAL_UART_Receive_DMA(&huart2, &usart_data[0][0], 2048);
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	printf("Error");
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   This file provides code for the configuration
  *          of the IWDG instances.
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
#include "iwdg.h"

/* USER CODE BEGIN 0 */
#include "stm32f1xx_hal_iwdg.h"
/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

// 直接操作寄存器初始化 IWDG，不依赖 HAL 库
// void HAL_IWDG_Init(void)
// {
//   // 解除 IWDG 保护
//   IWDG->KR = 0x5555;
  
//   // 预分频器 64
//   IWDG->PR = 0x02;
  
//   // 重装值 4095（溢出时间 ~6.5s）
//   IWDG->RLR = 0xFFF;
  
//   // 重载
//   IWDG->KR = 0xAAAA;
  
//   // 启动 IWDG
//   IWDG->KR = 0xCCCC;
// }

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */


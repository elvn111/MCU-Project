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
#include "cmsis_os.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "OLED.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void Set_Time(void);
void Read_Time(void);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//#if (__ARMCC_VERSION >= 6010050)
//__asm(".global __use_no_semihosting\n\t"); // ARMCC6
//#else
//#pragma import(__use_no_semihosting) // ARMCC5
//#endif

uint32_t a=1,b=5;
char ch_library[4][16] __attribute__((aligned(32))) = {0};//
int fputc(int ch, FILE *f){
  uint8_t temp[1] = {ch};
  HAL_UART_Transmit(&huart2, temp, 1, 0);
	if(ch=='\n'){
		if(a!=1){
			a=15;
		}else{
			a=0;
		}
	}else{
		ch_library[(b-1)%4][a-1]=ch;
		OLED_ShowChar(4,a,(char)ch);
	}
	if(a<15){
		a++;
	}else{
		OLED_Clear();
		b++;
		for(a=0;a<=15;a++){
			ch_library[(b-1)%4][a]=0;
		}
		a=1;
	}
	if(a==1){
		OLED_ShowString(1,1,ch_library[(b)%4]);
		OLED_ShowString(2,1,ch_library[(b+1)%4]);
		OLED_ShowString(3,1,ch_library[(b+2)%4]);
	}
		HAL_Delay(50);
    return ch;
}



uint16_t time_data[7]={2026,3,20,5,1,0,0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init();
	printf("[Debug]main:begin %d\n",HAL_GetTick());
	printf("[Debug]main:enter RTOS\n");
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	printf("[Debug]main:enter while %d \n",HAL_GetTick());
	//printf("Set PWM Compare:%d \n",1000);
  //__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1,1000);
  Set_Time();
  OLED_Clear();
	//OLED_Horizon(6,1);
  while (1)
  {
		//OLED_Horizon(16,((i++)%512)-256);
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Set_Time(void)
{
  RTC_TimeTypeDef RTC_Timestructure;
  RTC_DateTypeDef RTC_Datestructure;
  RTC_Datestructure.Year=time_data[0]-1900;
  RTC_Datestructure.Month=time_data[1];
  RTC_Datestructure.Date=time_data[2];
  RTC_Datestructure.WeekDay=time_data[3];
  
  RTC_Timestructure.Hours=time_data[4];
  RTC_Timestructure.Minutes=time_data[5];
  RTC_Timestructure.Seconds=time_data[6];
  
  HAL_RTC_SetDate(&hrtc,&RTC_Datestructure,RTC_FORMAT_BIN);
  HAL_RTC_SetTime(&hrtc,&RTC_Timestructure,RTC_FORMAT_BIN);
}

void Read_Time(void)
{
  RTC_TimeTypeDef RTC_Timestructure;
  RTC_DateTypeDef RTC_Datestructure;
  HAL_RTC_GetDate(&hrtc,&RTC_Datestructure,RTC_FORMAT_BIN);
  HAL_RTC_GetTime(&hrtc,&RTC_Timestructure,RTC_FORMAT_BIN);
  time_data[0]=RTC_Datestructure.Year+1900;
  time_data[1]=RTC_Datestructure.Month;
  time_data[2]=RTC_Datestructure.Date;
  time_data[3]=RTC_Datestructure.WeekDay;
  
  time_data[4]=RTC_Timestructure.Hours;
  time_data[5]=RTC_Timestructure.Minutes;
  time_data[6]=RTC_Timestructure.Seconds;

}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  printf("error");
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

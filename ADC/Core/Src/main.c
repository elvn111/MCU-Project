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
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "OLED.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TestON
//#define TestOFF

//#ifdef TestON
//	printf("");
//#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 1. ???????(????????)
#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t"); // ARMCC6
#else
#pragma import(__use_no_semihosting) // ARMCC5
#endif

// ÷ÿ∂®œÚ
uint32_t a=1,b=5;
char ch_library[4][16] __attribute__((aligned(32))) = {0};//
int fputc(int ch, FILE *f)
{
  uint8_t temp[1] = {ch};
  HAL_UART_Transmit(&huart2, temp, 1, 0);
		if(ch=='\n'){
			if(a!=1){
				a=15;
			}
			else{
				a=0;
			}
		}
		else{
			ch_library[(b-1)%4][a-1]=ch;
			OLED_ShowChar(4,a,(char)ch);
		}
		if(a==1)
		{
				int roll_base = (b-1) % 4;
				OLED_ShowString(1,1,ch_library[(roll_base+1)%4]);
				OLED_ShowString(2,1,ch_library[(roll_base+2)%4]);
				OLED_ShowString(3,1,ch_library[(roll_base+3)%4]);
		}
		
	if(a<15){
		a++;
	}
	else{
		OLED_Clear();
		b++;
		for(a=0;a<=15;a++){
			ch_library[(b-1)%4][a]=0;
		}
		a=1;
	}
		HAL_Delay(10);
    return ch;
}

// 3. ??:????????(??L6915E??)
// 3.1 ??_sys_exit(????????????)
void _sys_exit(int x)
{
    x = x; // ?????????
    while(1); // ???,??????
}

// 3.2 ??_sys_open(??????????)
typedef int FILEHANDLE; // ??:????FILEHANDLE?int??
FILEHANDLE _sys_open(const char *name, int openmode)
{
    (void)name;
    (void)openmode;
    return 0; // ?????
}

// 3.3 ??_ttywrch(??????????)
void _ttywrch(int ch)
{
    (void)ch; // ???,???????
}

// 3.4 ??:???????????????(????)
struct __FILE
{
    int handle;
    // ????(???)
};
FILE __stdout; // ??stdout,??printf???????


uint32_t num[2]={0};
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init();
	//OLED_ShowString(1,1,"KSKBL");
	printf("[Debug]main:begin\n");
	//OLED_ShowString(1,1,ch_library[0]);
	
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	printf("[Debug]main:entry while\n");
  while (1)
  {
		HAL_ADC_Start_DMA(&hadc1,(uint32_t *)num,2);
    //HAL_Delay(500);
		//printf("[Debug]main:begin\n");
		//HAL_Delay(500);
		//printf("[Debug]main:entry while\n");
    printf("c1:%d c2:%d\n",num[0],num[1]);
		//printf("[Debug]main:beg\n");
		
		//printf("%d\n",dsb++);
    //printf("%d %d %d %d",num[4],num[5],num[6],num[7]);

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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

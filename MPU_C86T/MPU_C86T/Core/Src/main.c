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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "IIC_Hardware.h"
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
		HAL_Delay(10);
    return ch;
}
/* USER CODE END PFP */
  typedef union{
    struct 
    {
      uint8_t Lbyte;
      uint8_t Hbyte;
    }unite;
    int16_t total;
  }Halfworld;

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
  uint8_t ab=0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  Halfworld X;
  Halfworld Y;
  Halfworld Z;
  float X_g ;
  float Y_g ;
  float Z_g ;
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	OLED_Init();
  //uint8_t a;
  
  //HAL_I2C_Master_Receive(&hi2c1,0xD075,&ab,1,10);
	printf("SI MA SHU DIAN %d\n",ab);
  /* USER CODE END 2 */
  IIC1_Init();
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  sqrt(X.total*X.total+Y.total*Y.total+Z.total*Z.total);
  while (1)
  {
    X.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_XOUT_H);
    X.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_XOUT_L);
    Y.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_YOUT_H);
    Y.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_YOUT_L);
    Z.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_ZOUT_H);
    Z.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_ZOUT_L);
    X_g= X.total / 16384.0f;
    Y_g= Y.total / 16384.0f;
    Z_g= Z.total / 16384.0f;
    printf("all a:%f \n",sqrt(X_g*X_g+Y_g*Y_g+Z_g*Z_g));
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

/* USER CODE BEGIN 4 */

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
  __disable_irq();
  printf("[Debug]:Error");
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

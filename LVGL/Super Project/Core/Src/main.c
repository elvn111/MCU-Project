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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "iwdg.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "IIC_Hardware.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include "OLED_Font.h"
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
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
uint8_t usart_data[128]={'1'};
uint8_t usart_data1[128]={'1'};
uint8_t usart_reflag =0;
uint8_t usart_reflag1 =0;
extern union {
	 uint8_t OLEDPicture_8[128][8];
	 uint64_t OLEDPicture_64[128];
 }OLEDPicture_total;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 重启原因枚举
typedef enum {
    RESET_POR = 0,    // 上电复位
    RESET_PIN,        // 复位脚复位
    RESET_IWDG,       // 独立看门狗 IWDG
    RESET_WWDG,       // 窗口看门狗 WWDG
    RESET_SOFT,       // 软件复位
    RESET_UNKNOWN     // 未知
} Reset_Cause_t;

Reset_Cause_t Reset_Cause = RESET_UNKNOWN;

const char Reset_Show[6][15]={
    "RESET_POR", 
    "RESET_PIN",        
    "RESET_IWDG",     
    "RESET_WWDG",      
    "RESET_SOFT",     
    "RESET_UNKNOWN"    
};
// 获取复位原因（必须在 HAL 初始化之前调用！）
void Check_Reset_Reason(void)
{
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        Reset_Cause = RESET_IWDG;  // IWDG 复位
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        Reset_Cause = RESET_WWDG;  // WWDG 复位
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        Reset_Cause = RESET_PIN;   // 复位按键
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        Reset_Cause = RESET_POR;   // 上电
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        Reset_Cause = RESET_SOFT;  // 软件复位
    }
    else
    {
        Reset_Cause = RESET_UNKNOWN;
    }

    // 清除所有复位标志（必须清！否则下次判断错误）
    __HAL_RCC_CLEAR_RESET_FLAGS();
}



//uint32_t a=1,b=5;
//char ch_library[4][16] __attribute__((aligned(32))) = {0};//
//int fputc(int ch, FILE *f){
//  uint8_t temp[1] = {ch};
//  HAL_UART_Transmit(&huart2, temp, 1, 0);
//	if(ch=='\n'){
//		if(a!=1){
//			a=15;
//		}else{
//			a=0;
//		}
//	}else{
//		ch_library[(b-1)%4][a-1]=ch;
//		OLED_ShowChar(4,a,(char)ch);
//	}
//	if(a<15){
//		a++;
//	}else{
//		OLED_Clear();
//		b++;
//		for(a=0;a<=15;a++){
//			ch_library[(b-1)%4][a]=0;
//		}
//		a=1;
//	}
//	if(a==1){
//		OLED_ShowString(1,1,ch_library[(b)%4]);
//		OLED_ShowString(2,1,ch_library[(b+1)%4]);
//		OLED_ShowString(3,1,ch_library[(b+2)%4]);
//	}
//		HAL_Delay(50);
//    return ch;
//}

//小字版打印
const uint8_t rowsum=8; 
uint32_t a=1,b=5;
char ch_library[rowsum][16] __attribute__((aligned(32))) = {0};//
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
		ch_library[(b-1)%rowsum][a-1]=ch;
		OLED_ShowMinChar(rowsum,a,(char)ch);
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
			OLED_ShowMinString(show_num+1,1,ch_library[(b+show_num)%rowsum]);
		}
	}
		HAL_Delay(50);
    return ch;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	__enable_irq();//��Ϊappʱ���bootload����
  Check_Reset_Reason();
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_I2C2_Init();
  //MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
	IIC1_Init();
  //MX_IWDG_Init();
  __HAL_RCC_PWR_CLK_ENABLE();
  lprintf("[App]:Freertos begin %d\n",HAL_GetTick());
  printf("Reset for %s \n",Reset_Show[Reset_Cause]);
  /* USER CODE END 2 */

  /* Init scheduler */
  //osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  //MX_FREERTOS_Init();

  /* Start scheduler */
  //osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	//OLED_Clear();
	int a=32;
	int a_a=1;
	int b=0;
	int b_b=6;
	int x=0,y=0,angle=0,middle=0;
	int x1_r=0,y1_r=0,x2_r=0,y2_r=0,x3_r=0,y3_r=0,x4_r=0,y4_r=0;
	int fps,count;
	int biankuang=4;
  while (1)
  {
		memset(OLEDPicture_total.OLEDPicture_8[0],0x00,1024);
		//OLED_AMPDrawARatateWord(16,16,b,'B',OLED_White,1);
//		OLED_DrawARatateString(x,y,angle,(char *)usart_data1+1,OLED_White);
//		rotate_point(x+4*strlen((char *)usart_data1+1),y+8,angle,x,y,&x1_r,&y1_r);
//		rotate_point(x+4*strlen((char *)usart_data1+1),y+8,angle,x,y+16,&x2_r,&y2_r);
//		rotate_point(x+4*strlen((char *)usart_data1+1),y+8,angle,x+8*strlen((char *)usart_data1+1),y+16,&x3_r,&y3_r);
//		rotate_point(x+4*strlen((char *)usart_data1+1),y+8,angle,x+8*strlen((char *)usart_data1+1),y,&x4_r,&y4_r);
//		OLED_DrawAquard(x1_r,y1_r,x2_r,y2_r,x3_r,y3_r,x4_r,y4_r,OLED_Contr);
//		OLEDMin_DrawARatateNum(0,56,0,fps,2,OLED_White);
//		
//		OLED_DrawAquard(0,0,0,64,128,64,128,0,OLED_White);
//		OLED_DrawAquard(0,0,0,0,128,64,128,64,OLED_White);
//		OLED_DrawAquard(0+biankuang,0+2*biankuang,0+biankuang,64-biankuang,128-biankuang,64-biankuang,128-biankuang,0+2*biankuang,OLED_Black);
//		OLED_DrawAquard(0,0,0,0,128,64,128,64,OLED_White);
		OLED_DrawAquard(8,0,9,0,9,64,9,64,OLED_White);
		for(int i=0;i<5;i++)
		{
			OLED_DrawAquard(8,8+i*16,9,9+i*16,128,9+i*16,128,8+i*16,OLED_White);
		}
		
		//OLED_DrawAquard(8,24,9,25,128,25,128,24,OLED_White);
		
//		OLED_DrawARound(64,36,20,0,OLED_White);
//		OLED_DrawAPolygon(64,36,4,30,-90,OLED_White);
//		OLED_DrawARound(64,36,20,19,OLED_White);
//		OLED_DrawARound(64,36,30,29,OLED_White);
		//OLED_DrawAString(32,48,"Warrning",OLED_White);
//		OLEDMin_DrawARatateWord(60,9,90,'M',OLED_Contr);
//		OLEDMin_DrawARatateWord(44,16,40,'B',OLED_Contr);
//		OLEDMin_DrawARatateWord(78,18,140,'W',OLED_Contr);
		
//		OLED_DrawARound(32,40,4,OLED_White);
//		OLED_DrawARound(96,40,4,OLED_White);
//		OLED_DrawAquard(32,36,
//										96,36,
//										96,44,
//										32,44,OLED_White);
		
//		OLED_DrawARound(32,40,2,OLED_Black);
//		OLED_DrawARound(96,40,2,OLED_Black);
//		OLED_DrawAquard(32,38,
//										96,38,
//										96,42,
//										32,42,OLED_Black);
		
//		OLED_DrawARound(a,40,2,OLED_White);
//		OLED_DrawARound(96,40,2,OLED_White);
//		OLED_DrawAquard(a ,38,
//										96,38,
//										96,42,
//										a ,42,OLED_White);
		fps=1000/(HAL_GetTick()-count);
		count=HAL_GetTick();

		OLED_ShowPicture();
		if(usart_data1[0]=='t')
		{
			y--;
		}else if(usart_data1[0]=='l'){
			x--;
		}else if(usart_data1[0]=='m'){
			
		}else if(usart_data1[0]=='r'){
			x++;
		}else if(usart_data1[0]=='b'){
			y++;
		}else if(usart_data1[0]=='a'){
			angle++;
		}else if(usart_data1[0]=='c'){
			angle--;
		}
		
		usart_data1[0]=0;
		if(a>96||a<32)
		{
			a_a=-a_a;
		}
		a+=a_a;
		if(b>20||b<0)
		{
			b_b=-b_b;
		}
		b+=b_b;
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
		//HAL_Delay(50);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

// IWDG_HandleTypeDef IWDGHandler;
// void MX_IWDG (void)
// {
//   IWDGHandler.Instance=IWDG;
//   IWDGHandler.Init.Prescaler=IWDG_PRESCALER_64;
//   IWDGHandler.Init.Reload=4095;
//   if (HAL_IWDG_Init(&IWDGHandler) != HAL_OK)
//   {
//     Error_Handler();
//   }
// }

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_0)
    {
        //HAL_NVIC_SystemReset();
    }
}
extern DMA_HandleTypeDef hdma_uart2;
//uint8_t usart_data[128]={0};

uint8_t usart_Echo=91;
void USART2_IRQHandler(void)
{
  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_RXNE)!=RESET)
  {
    usart_reflag=1;
    __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);
    __HAL_UART_DISABLE_IT(&huart2,UART_IT_RXNE);
    uint8_t temp = huart2.Instance->DR;
    usart_data[0]=temp;
    HAL_UART_Receive_DMA(&huart2,usart_data+1,127);
    __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);
    
  }
  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE)!=0)
  {
    usart_reflag1=1;
		HAL_UART_AbortReceive(&huart2);
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
    __HAL_UART_DISABLE_IT(&huart2,UART_IT_IDLE);
    HAL_UART_Transmit(&huart2,&usart_Echo,1,1);
    __HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);
  }
  //HAL_UART_IRQHandler(&huart2);
}


void DMA1_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */
	
  /* USER CODE END DMA1_Channel1_IRQn 0 */
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
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
	printf("Error");
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

/* USER CODE BEGIN Header */
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "semphr.h"
#include "OLED.h"
#include "BNO08X.h"
#include <stdio.h>
#include <string.h>
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_size = 384 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};
/* Definitions for OLED_State */
osMutexId_t OLED_StateHandle;
const osMutexAttr_t OLED_State_attributes = {
  .name = "OLED_State"
};
/* Definitions for myMutex02 */
osMutexId_t myMutex02Handle;
const osMutexAttr_t myMutex02_attributes = {
  .name = "myMutex02"
};
/* Definitions for myBinarySem01 */
osSemaphoreId_t myBinarySem01Handle;
const osSemaphoreAttr_t myBinarySem01_attributes = {
  .name = "myBinarySem01"
};
/* Definitions for myCountingSem01 */
osSemaphoreId_t myCountingSem01Handle;
const osSemaphoreAttr_t myCountingSem01_attributes = {
  .name = "myCountingSem01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void    App_OledLine(uint8_t line, const char *text);
static uint8_t App_FmtFixed1(char *buf, float value, uint8_t show_sign);
static uint8_t App_FmtUint(char *buf, uint32_t value);
static void    App_ShowAttitude(const BNO08X_Data_t *data);
static void    App_PrintAttitude(const BNO08X_Data_t *data);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of OLED_State */
  OLED_StateHandle = osMutexNew(&OLED_State_attributes);

  /* creation of myMutex02 */
  myMutex02Handle = osMutexNew(&myMutex02_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of myBinarySem01 */
  myBinarySem01Handle = osSemaphoreNew(1, 1, &myBinarySem01_attributes);

  /* creation of myCountingSem01 */
  myCountingSem01Handle = osSemaphoreNew(2, 0, &myCountingSem01_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (1, sizeof(uint16_t), &myQueue01_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(StartTask02, NULL, &myTask02_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(StartTask03, NULL, &myTask03_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  const BNO08X_Data_t *data;
  uint32_t t_oled = 0u;
  uint32_t t_print = 0u;
  uint32_t now;

  (void)argument;

  /* 初始化 BNO08X（I2C 或 UART-RVC，由 BNO08X.h 里的 BNO08X_MODE 选择） */
  BNO08X_Init();

  /* Infinite loop */
  for(;;)
  {
    /* 高频服务：读取并解析传感器数据（内部无阻塞等待） */
    BNO08X_Service();
    now  = HAL_GetTick();
    data = BNO08X_GetData();

    /* OLED 每 200ms 刷新一次（5Hz），显示航向角/俯仰角/横滚角；
       串口每 1s 打印一次。两者错开执行，避免一次循环里连续长时间
       阻塞（RVC 模式下 DMA 环形缓冲只有约 89ms 余量）。 */
    if ((uint32_t)(now - t_oled) >= 200u)
    {
      t_oled = now;
      App_ShowAttitude(data);
    }
    else if ((uint32_t)(now - t_print) >= 1000u)
    {
      t_print = now;
      App_PrintAttitude(data);
    }

    osDelay(2);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    // xSemaphoreTake(myBinarySem01Handle,portMAX_DELAY);
    // printf("Something\n");
    // HAL_Delay(1000);
    // OLED_Clear();
    // //vTaskSuspend(myTask02Handle);
    // xSemaphoreGive(myBinarySem01Handle);
    
    // vTaskDelay(1);
    osDelay(1);
  }
  /* USER CODE END StartTask03 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* 在 OLED 上显示一整行（不足 16 列补空格，防止刷新后残留旧字符） */
static void App_OledLine(uint8_t line, const char *text)
{
  char    buf[17];
  uint8_t i = 0u;

  while ((i < 16u) && (text[i] != '\0'))
  {
    buf[i] = text[i];
    i++;
  }
  while (i < 16u)
  {
    buf[i] = ' ';
    i++;
  }
  buf[16] = '\0';

  OLED_ShowString(line, 1, buf);
}

/* 把定点数格式化成 "123.4" / "-45.6" / "+12.3"（1 位小数），返回字符数 */
static uint8_t App_FmtFixed1(char *buf, float value, uint8_t show_sign)
{
  char     tmp[12];
  uint8_t  n = 0u;
  uint8_t  t = 0u;
  int32_t  scaled;
  uint32_t ipart;
  uint32_t fpart;

  if (value < 0.0f)
  {
    buf[n++] = '-';
    value = -value;
  }
  else if (show_sign != 0u)
  {
    buf[n++] = '+';
  }

  scaled = (int32_t)(value * 10.0f + 0.5f);
  ipart  = (uint32_t)(scaled / 10);
  fpart  = (uint32_t)(scaled % 10);

  do
  {
    tmp[t++] = (char)('0' + (ipart % 10u));
    ipart /= 10u;
  } while (ipart > 0u);

  while (t > 0u)
  {
    buf[n++] = tmp[--t];
  }
  buf[n++] = '.';
  buf[n++] = (char)('0' + fpart);
  buf[n]   = '\0';

  return n;
}

/* 无符号整数转字符串，返回字符数 */
static uint8_t App_FmtUint(char *buf, uint32_t value)
{
  char    tmp[12];
  uint8_t n = 0u;
  uint8_t t = 0u;

  do
  {
    tmp[t++] = (char)('0' + (value % 10u));
    value /= 10u;
  } while (value > 0u);

  while (t > 0u)
  {
    buf[n++] = tmp[--t];
  }
  buf[n] = '\0';

  return n;
}

/* OLED 显示：航向角 + 俯仰 + 横滚 + 状态 */
static void App_ShowAttitude(const BNO08X_Data_t *data)
{
  char        line[17];
  uint8_t     n;
  const char *dir;

  if ((data == NULL) || (data->valid == 0u) || (data->online == 0u))
  {
    if ((data != NULL) && (data->mode == 2u))
    {
      App_OledLine(1, "BNO08X  RVC");
      App_OledLine(2, "NO DATA");
      App_OledLine(3, "CHECK RX(PB11)");
      App_OledLine(4, "PS0=3V3 115200?");
    }
    else
    {
      App_OledLine(1, "BNO08X  I2C");
      App_OledLine(2, "NO DATA");
      App_OledLine(3, "CHECK SCL/SDA");
      App_OledLine(4, "ADDR 4A/4B PULL?");
    }
    return;
  }

  /* 第 1 行：HDG 123.4 NW */
  n = 0u;
  line[n++] = 'H';
  line[n++] = 'D';
  line[n++] = 'G';
  line[n++] = ' ';
  n = (uint8_t)(n + App_FmtFixed1(&line[n], data->yaw, 0u));
  line[n++] = ' ';
  dir = BNO08X_GetCompassDir(data->yaw);
  while ((*dir != '\0') && (n < 16u))
  {
    line[n++] = *dir++;
  }
  line[n] = '\0';
  App_OledLine(1, line);

  /* 第 2 行：PIT +12.3 */
  n = 0u;
  line[n++] = 'P';
  line[n++] = 'I';
  line[n++] = 'T';
  line[n++] = ' ';
  n = (uint8_t)(n + App_FmtFixed1(&line[n], data->pitch, 1u));
  line[n] = '\0';
  App_OledLine(2, line);

  /* 第 3 行：ROL -45.6 */
  n = 0u;
  line[n++] = 'R';
  line[n++] = 'O';
  line[n++] = 'L';
  line[n++] = ' ';
  n = (uint8_t)(n + App_FmtFixed1(&line[n], data->roll, 1u));
  line[n] = '\0';
  App_OledLine(3, line);

  /* 第 4 行：ACC:3 25Hz */
  n = 0u;
  line[n++] = 'A';
  line[n++] = 'C';
  line[n++] = 'C';
  line[n++] = ':';
  line[n++] = (char)('0' + (data->accuracy & 0x0Fu));
  line[n++] = ' ';
  n = (uint8_t)(n + App_FmtUint(&line[n], (uint32_t)data->rate_hz));
  line[n++] = 'H';
  line[n++] = 'z';
  line[n]   = '\0';
  App_OledLine(4, line);
}

/* 串口打印：方便用串口助手（9600 8N1）查看姿态数据 */
static void App_PrintAttitude(const BNO08X_Data_t *data)
{
  char yaw_txt[12];
  char pit_txt[12];
  char rol_txt[12];

  if ((data == NULL) || (data->valid == 0u))
  {
    printf("[BNO08X] waiting data... mode=%d\r\n",
           (data != NULL) ? (int)data->mode : 0);
    return;
  }

  App_FmtFixed1(yaw_txt, data->yaw, 0u);
  App_FmtFixed1(pit_txt, data->pitch, 1u);
  App_FmtFixed1(rol_txt, data->roll, 1u);

  /* 打印格式尽量短：RVC 模式 9600 波特率下约 45ms 发完 */
  printf("[BNO08X] H=%s P=%s R=%s\r\n", yaw_txt, pit_txt, rol_txt);
}

/* USER CODE END Application */

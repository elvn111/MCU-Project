#include "gpio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "KEY.h"

extern EventGroupHandle_t EventGroupHandle;
extern uint8_t usart_data[128];
extern uint8_t usart_data1[128];

/**
  * @brief  按键检测
  */
void ReadKeyTask (void)
{
    uint16_t Key_num = 0;

    // 读取物理按键状态
    if (!HAL_GPIO_ReadPin(Button_GPIO_Port, Button1_Pin))
    {
        Key_num |= KEY_CONFIRM;  // bit0: 确认
    }
    if (!HAL_GPIO_ReadPin(Button_GPIO_Port, Button2_Pin))
    {
        Key_num |= KEY_UP;       // bit1: 上
    }
    if (!HAL_GPIO_ReadPin(Button_GPIO_Port, Button3_Pin))
    {
        Key_num |= KEY_DOWN;     // bit3: 下
    }
    if (!HAL_GPIO_ReadPin(Button_GPIO_Port, Button4_Pin))
    {
        Key_num |= KEY_EXIT;     // bit4: 退出或复位
    }

    // 等待按键释放
    while (!HAL_GPIO_ReadPin(Button_GPIO_Port, Button1_Pin) ||
           !HAL_GPIO_ReadPin(Button_GPIO_Port, Button2_Pin) ||
           !HAL_GPIO_ReadPin(Button_GPIO_Port, Button3_Pin) ||
           !HAL_GPIO_ReadPin(Button_GPIO_Port, Button4_Pin))
    {
        ;
    }

    // 处理串口输入
    if (usart_data1[0] == 't')
    {
        Key_num |= KEY_UP;       // 上
    }
    else if (usart_data1[0] == 'l')
    {
        Key_num |= KEY_CONFIRM;  // 确认
    }
    else if (usart_data1[0] == 'm')
    {
        Key_num |= KEY_MIDLE;
    }
    else if (usart_data1[0] == 'r')
    {
        Key_num |= KEY_EXIT;     // 退出
    }
    else if (usart_data1[0] == 'b')
    {
        Key_num |= KEY_DOWN;     // 下
    }
    else if (usart_data1[0] == 'a')
    {
        Key_num |= KEY_A;
    }
    else if (usart_data1[0] == 'c')
    {
        // C键，无操作
    }

    usart_data1[0] = 0;

    // 设置事件组
    xEventGroupSetBits(EventGroupHandle, Key_num);
}

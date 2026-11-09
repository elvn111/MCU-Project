#ifndef __ELVNRTOS_H__
#define __ELVNRTOS_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stm32f10x.h>

#define MAX_TASK_NUM 20-1
#define TASK_PRIORITY_MAX 10-1
//#define TASK_PC 

typedef int task_retrun;
typedef int task_parameter;
typedef void * handle;


typedef struct{
    int Task_Priority;//任务优先级 0~10 数字越大优先级越高
    uint32_t * Task_SP;//任务栈顶指针
    uint32_t * Task_SP_MAX;//任务最大栈顶指针
    uint32_t * Task_State;//任务状态 0就绪 1运行 2阻塞 3挂起
    uint32_t * Task_BP;//任务基址指针
}Task_Sign;


typedef struct{
    handle * Handle;//任务句柄
    task_retrun (*Fuction)(task_parameter);//任务函数
    char * Task_Name;//任务名称
    int Size;//任务栈大小
    int Task_Priority;//任务优先级 0~10 数字越大优先级越高
}Task_InitStruct;


void ELVNRTOS_IdleTask(void);

uint8_t ELVNRTOS_Scheduler(void);
uint8_t ELVNRTOS_Init(Task_InitStruct * a);

void TIM2_IRQHandler(void);
uint8_t ELVNRTOS_Creat(Task_InitStruct * a);

#endif /* __ELVNRTOS_H__ */
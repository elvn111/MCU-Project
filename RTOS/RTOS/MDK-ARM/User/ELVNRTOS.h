#ifndef __ELVNRTOS_H__
#define __ELVNRTOS_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stm32f1xx.h"
//#include "stm32f1xx_hal.h"
#include "tim.h"

#define MAX_TASK_NUM 20-1
#define TASK_PRIORITY_MAX 10-1
#define Heap_MAX 4096
#define PILE_SIZE 32
#define HEAP_SIZE_FOR_PILE 32
#define ERTOS_Delay_MAx 9178
// #define BUSY 1
// #define IDLE 0
//#define TASK_PC 

typedef int task_retrun;
typedef int task_parameter;
typedef void * handle;

typedef enum
{
    IDLE =0,
    BUSY =1
}HEAP_STATE;

typedef struct {
    uint32_t * SP;              // 当前栈指针
    uint32_t * SP_MAX;         // 栈底
    uint32_t * BP;             // 栈顶
    uint8_t State;              // 0=就绪, 1=运行
    uint8_t Priority;
} Task_Sign;

typedef struct{
    handle * Handle;//任务句柄
    task_retrun (*Fuction)(task_parameter);//任务函数
    char * Name;//任务名称
    int Size;//任务栈大小
    int Priority;//任务优先级 0~10 数字越大优先级越高
}Task_InitStruct;

task_retrun ELVNRTOS_IdleTask(task_parameter);
void ETask_Scheduler(void);
void ETask_StartScheduler(void);
uint8_t ETask_Creat(Task_InitStruct * a);


uint32_t * Heap_Insert(uint16_t size);
uint8_t ELVNRTOS_Init(void);

void TIM1_UP_IRQHandler(void);
__asm void PendSV_Handler(void);

void Etask_Delay(uint32_t num);
void ETask_Yield(void);

#endif /* __ELVNRTOS_H__ */
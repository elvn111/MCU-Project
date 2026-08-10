#ifndef __ELVNRTOS_H__
#define __ELVNRTOS_H__

<<<<<<< HEAD

#include "OLED.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tim.h"

typedef void(*TaskFunction)(void* point);

/* 链表节点：内存来自静态数组 node_pool[] */
typedef struct TaskControlBlock {
    uint32_t* sp;              // 当前栈指针
    uint32_t size;
    uint32_t* stack_base;
    uint32_t* stack_top;   /* 栈底（分配时记录） */
    struct TaskControlBlock* prev;     /* 前驱节点 */
    struct TaskControlBlock* next;     /* 后继节点 */
    uint8_t priority;          /* 所属优先级（分配时写入，释放时据此定位队列） */
    uint16_t state;              
    uint8_t task_id;
} TaskControlBlock;
=======
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
>>>>>>> 83d35703597c9c86b0322e80814e6b888f7b0cdc

void TIM1_UP_IRQHandler(void);
__asm void PendSV_Handler(void);

<<<<<<< HEAD
uint8_t ETask_Creat(TaskFunction taskfunction,uint8_t priority,uint32_t size);
uint8_t ETask_StartScheduler(void);
uint8_t ELVNRTOS_Init(void);
void ELVNRTOS_Delete(TaskControlBlock *task);
// void Etask_Delay(uint32_t num);
void Etask_Delay(uint32_t num);
void ETask_Yield(void);
void Heap_Release(uint32_t *stack_base, uint32_t* stack_top);
=======
void Etask_Delay(uint32_t num);
void ETask_Yield(void);

>>>>>>> 83d35703597c9c86b0322e80814e6b888f7b0cdc
#endif /* __ELVNRTOS_H__ */
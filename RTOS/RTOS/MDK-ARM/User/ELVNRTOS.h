#ifndef __ELVNRTOS_H__
#define __ELVNRTOS_H__


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

void TIM1_UP_IRQHandler(void);
__asm void PendSV_Handler(void);

uint8_t ETask_Creat(TaskFunction taskfunction,uint8_t priority,uint32_t size);
uint8_t ETask_StartScheduler(void);
uint8_t ELVNRTOS_Init(void);
void ELVNRTOS_Delete(TaskControlBlock *task);
// void Etask_Delay(uint32_t num);
void Etask_Delay(uint32_t num);
void ETask_Yield(void);
void Heap_Release(uint32_t *stack_base, uint32_t* stack_top);
#endif /* __ELVNRTOS_H__ */
#include "ELVNRTOS.h"


#define PRIORITY_COUNT   10    
#define TASK_BLOCK_NUM   256
#define TASK_BLOCK_SIZE  8
#define TASK_HEAP_SIZE (TASK_BLOCK_NUM * TASK_BLOCK_SIZE)

#define ENTER_CRITICAL()  __disable_irq()
#define EXIT_CRITICAL()   __enable_irq()

extern TIM_HandleTypeDef htim1;


typedef enum {
    TASK_STATE_READY = 0,
    TASK_STATE_SUSPENDED = 9999
} TaskState;

typedef enum {
    STATE_FREE = 0,   /* 空闲：可分配 */
    STATE_BUSY = 1    /* 占用：已入队 */
} State;

__attribute__((__align(4)))uint8_t task_heap[TASK_BLOCK_NUM][TASK_BLOCK_SIZE];
static State task_block_state[TASK_BLOCK_NUM];

/* ---- 按优先级分链的就绪队列 ---- */
static TaskControlBlock* ready_head[PRIORITY_COUNT]={0};   /* 各优先级队列头指针 */
static TaskControlBlock* ready_tail[PRIORITY_COUNT]={0};   /* 各优先级队列尾指针 */
static uint16_t  ready_count[PRIORITY_COUNT];  /* 各优先级队列节点计数 */
static uint16_t  ready_total = 0;              /* 全部队列节点总数 */
uint16_t Task_ID = 0;

TaskControlBlock * Task_Current_TCB;

//时间片轮转绑定于定时器
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GET_ITSTATUS(&htim1, TIM_IT_UPDATE) != RESET)
    {
        // 清除中断标志
        TIM_GET_CLEAR_IT(&htim1, TIM_IT_UPDATE);
        // 调度
        ETask_Yield();
        //SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    }
}

//异常句柄来实现任务切换
__asm void PendSV_Handler(void)
{
    extern Task_Current_TCB;        // 外部引用：当前正在运行的任务TCB指针
	extern ETask_Scheduler;  // 外部引用：调度函数，选择下一个要运行的任务

	PRESERVE8                   // 【指令】：告诉编译器保持8字节栈对齐，Keil必须加

	/* ==============================
	 * 第一步：保存当前任务的 R4~R11
	 * ============================== */
	mrs r0, psp                 // 【指令】MRS：读取PSP寄存器 → R0 = PSP（任务栈指针）
	isb                         // 【指令】ISB：指令同步屏障，保证流水线干净

	ldr	r3, =Task_Current_TCB        // 【指令】LDR：把变量pxCurrentTCB的地址 → 加载到 R3
	ldr	r2, [r3]                // 【指令】LDR：间接寻址，R2 = *R3 = 当前任务TCB的首地址

	stmdb r0!, {r4-r11}         // 【指令】STMDB：多寄存器存储
                                // 把 R4~R11 依次压入 R0指向的栈（任务栈）
                                // 同时 R0 自动递减（DB=Decrement Before）
	str r0, [r2]                // 【指令】STR：把更新后的R0（新PSP）→ 存入TCB的第一个成员（sp）

	/* ==============================
	 * 第二步：保护现场，调用C语言调度器
	 * ============================== */
	stmdb sp!, {r3, r14}        // 【指令】STMDB：把 R3、LR 压入 MSP（中断栈）
	mov r0, #10
	msr basepri, r0             // 【指令】MSR：把 R0 → 写入 BASEPRI
                                // 作用：关闭低、中优先级中断，只允许高优先级中断
	dsb                         // 【指令】DSB：数据同步屏障
	isb                         // 【指令】ISB：指令同步屏障
	bl ETask_Scheduler       // 【指令】BL：调用C函数
                                // 执行后 pxCurrentTCB = 下一个任务
	mov r0, #0
	msr basepri, r0             // 【指令】MSR：BASEPRI=0，打开所有中断
	ldmia sp!, {r3, r14}        // 【指令】LDMIA：从中断栈恢复 R3、LR

	/* ==============================
	 * 第三步：恢复下一个任务的 R4~R11
	 * ============================== */
	ldr r1, [r3]                // 【指令】LDR：R1 = *R3 = 新任务TCB地址
	ldr r0, [r1]                // 【指令】LDR：R0 = 新任务TCB->sp（保存的PSP）

	ldmia r0!, {r4-r11}         // 【指令】LDMIA：多寄存器加载
                                // 从 R0指向的栈 弹出数据 → R4~R11
                                // R0 自动递增
	msr psp, r0                 // 【指令】MSR：把新栈指针R0 → 写入PSP
	isb                         // 【指令】ISB：指令同步

	bx r14                      // 【指令】BX：跳转到LR指向的地址
                                // 作用：退出中断，硬件自动弹8个寄存器，运行新任务
	nop                         // 空指令，对齐用
}

void test_A(void){
    OLED_Clear();
    OLED_ShowNum(1,1,__get_BASEPRI(),10);
    OLED_ShowNum(3,1,__get_PSP(),10);
    OLED_ShowNum(4,1,__HAL_TIM_GET_COUNTER(&htim1),10);
}

void test_B(void){
    test_A();
    OLED_ShowNum(1,1,__get_BASEPRI(),10);
    OLED_ShowNum(3,1,__get_PSP(),10);
    OLED_ShowNum(4,1,__HAL_TIM_GET_COUNTER(&htim1),10);
}

//空闲任务
void ELVNRTOS_IdleTask(void* a){
    while(1){
        // OLED_ShowNum(1,1,(uint32_t)Task_Sign_Table[0].SP,10);
        // OLED_ShowNum(2,1,(uint32_t)Task_Sign_Table[1].SP,10);
        // OLED_ShowNum(3,1,(uint32_t)Task_Sign_Table[2].SP,10);
        // OLED_ShowNum(4,1,(uint32_t)Task_Sign_Table[3].SP,10);
        test_B();
        //OLED_Clear();
        //Etask_Delay(10);
        //__WFI();
    }
}

void * Heap_Allocate(uint16_t size){
    if (size == 0)return 0;
    int num = (size + TASK_BLOCK_SIZE - 1) / TASK_BLOCK_SIZE;
    int count = 0;
    int block_head;
    uint32_t* addr = 0;
    ENTER_CRITICAL();
    for (int i = 0;i < TASK_BLOCK_NUM;i++){
        if (task_block_state[i] == STATE_BUSY){
            count = 0;
        }
        else if (task_block_state[i] == STATE_FREE){
            if (count == 0){
                addr = (uint32_t*)&task_heap[i][0];
            }
            count++;
            if (count == num){
                for (int j = 0;j < num;j++){
                    task_block_state[i-j] = STATE_BUSY;
                }
                EXIT_CRITICAL();
                return addr;
            }
        }
    }
    EXIT_CRITICAL();
    return 0;
}
uint8_t Heap_Release(uint8_t * addr,uint16_t size)
{
    uint32_t byte_offset;
    uint32_t block_index;
    uint32_t block_count;
    uint32_t i;
    ENTER_CRITICAL();
    /* 参数校验：node 和栈底都不能为空 */
    if (addr == NULL ) {
        EXIT_CRITICAL();
        return 0;
    }

    if (size == 0) {
        EXIT_CRITICAL();
        return 0;
    }

    byte_offset = (uint32_t)((uint8_t*)addr - (uint8_t*)task_heap);
    if (byte_offset >= TASK_HEAP_SIZE || (byte_offset % TASK_BLOCK_SIZE) != 0) {
        EXIT_CRITICAL();
        return 0;  
    }

    block_index = byte_offset / TASK_BLOCK_SIZE;
    block_count = (size + TASK_BLOCK_SIZE - 1) / TASK_BLOCK_SIZE;

    if (block_index + block_count > TASK_BLOCK_NUM) {
        EXIT_CRITICAL();
        return 0;
    }

    if (task_block_state[block_index] != STATE_BUSY) {
        EXIT_CRITICAL();
        return 0;
    }

    for (i = 0; i < block_count; i++) {
        task_block_state[block_index + i] = STATE_FREE;
    }

    memset((uint8_t*)addr, 0, TASK_BLOCK_SIZE * block_count);
    EXIT_CRITICAL();
    return 1;
}


/**
 * @brief  从节点池分配一个节点，并挂入指定优先级的就绪队列尾部
 * @param  priority 目标优先级，取值范围 [0, PRIORITY_COUNT)
 * @return 成功返回节点指针；节点池已满或优先级非法返回 NULL
 */
TaskControlBlock* NodePool_Allocate(uint8_t priority){
    uint32_t i;
    TaskControlBlock* node;
    ENTER_CRITICAL();
    /* 边界检查：优先级必须小于队列数量上限，否则数组越界 */
    if (priority >= PRIORITY_COUNT) {
        EXIT_CRITICAL();
        return NULL;
    }

    node=Heap_Allocate(sizeof(TaskControlBlock));
    if(node==NULL){
        EXIT_CRITICAL();
        return NULL;
    }

    node->priority = priority;
    /* 清空指针域，避免复用历史数据产生悬空引用 */
    node->prev = NULL;
    node->next = NULL;

    /* 入队：追加到对应优先级队列的尾部 */
    if (ready_count[priority] == 0) {
        ready_head[priority] = node;
        ready_tail[priority] = node;
    } else {
        ready_tail[priority]->next = node;
        node->prev = ready_tail[priority];
        ready_tail[priority] = node;
    }

    /* 更新队列计数与总计数 */
    ready_count[priority]++;
    ready_total++;
    EXIT_CRITICAL();
    return node;

    /* 节点池已满，分配失败 */
    EXIT_CRITICAL();
    return NULL;
}


/**
 * @brief  将节点从就绪队列中摘除，并归还至节点池
 * @param  node 待释放节点指针
 * @return 成功返回 1；参数非法、重复释放或队列为空返回 0
 * @note   本函数不适用于释放不属于本节点池的指针
 */
int NodePool_Release(TaskControlBlock* node)
{
    uint8_t priority;

    /* 参数合法性校验：任一条件不满足则直接拒绝 */
    ENTER_CRITICAL();
    if (ready_total == 0) {                          /* 队列为空 */
        EXIT_CRITICAL();
        return 0;
    }
    if (node == NULL) {                              /* 空指针 */
        EXIT_CRITICAL();
        return 0;
    }

    /* 读取节点所属优先级 */
    priority = node->priority;

    if (ready_count[priority] == 1) {
        ready_head[priority] = NULL;
        ready_tail[priority] = NULL;
    } else if (node == ready_head[priority]) {
        ready_head[priority] = node->next;
        ready_head[priority]->prev = NULL;
    } else if (node == ready_tail[priority]) {
        ready_tail[priority] = node->prev;
        ready_tail[priority]->next = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    Heap_Release((uint8_t*)node,sizeof(TaskControlBlock));
    /* 统一收尾：清除指针域、归还节点、更新计数 */
    node->prev = NULL;
    node->next = NULL;
    ready_count[priority]--;
    ready_total--;
    EXIT_CRITICAL();
    return 1;
}

uint8_t ETask_Creat(TaskFunction taskfunction,uint8_t priority,uint32_t size)
{
    uint32_t* stack_base;
    uint32_t* stack_top;
    TaskControlBlock *node;
    ENTER_CRITICAL();
    stack_base=(uint32_t *)Heap_Allocate(size);
    stack_top=(uint32_t * )((uint8_t *)stack_base+((size + TASK_BLOCK_SIZE - 1) / TASK_BLOCK_SIZE)*TASK_BLOCK_SIZE);
    if(stack_base==NULL)
    {
        EXIT_CRITICAL();
        return 0;
    }
    if ((uint32_t*)stack_top >(uint32_t*)&task_heap[TASK_BLOCK_NUM-1][TASK_BLOCK_SIZE-1]){
        EXIT_CRITICAL();
        return 0;
    }
    node=NodePool_Allocate(priority);
    if (node == 0) {
        Heap_Release((uint8_t *)stack_base, size);
        EXIT_CRITICAL();
        return 0;
    }
    node->size = size;
    node->stack_base = stack_base;
    node->stack_top = stack_top;
    node->task_id = Task_ID;
    Task_ID++;
    uint32_t* sp = stack_top;

    *--sp = 0x01000000;              // xPSR (Thumb位=1)
    *--sp = (uint32_t)taskfunction;    // PC (任务入口)
    *--sp = 0xFFFFFFFD;              // LR (EXC_RETURN)
    *--sp = 0;                       // R12
    *--sp = 0;                       // R3
    *--sp = 0;                       // R2
    *--sp = 0;                       // R1
    *--sp = 0;                       // R0

    *--sp = 0;                       // R4
    *--sp = 0;                       // R5
    *--sp = 0;                       // R6
    *--sp = 0;                       // R7
    *--sp = 0;                       // R8
    *--sp = 0;                       // R9
    *--sp = 0;                       // R10
    *--sp = 0;                       // R11
    // 保存到任务控制块
    node->sp = sp;      // 指向 R0
    node->state = 0;
    //创建成功
    EXIT_CRITICAL();
    return 1;
}

uint8_t ETask_StartScheduler(void){
    if(ready_total == 0) return 0;
    uint32_t first_sp ;
    first_sp = (uint32_t)ready_head[0]->sp;
    // 设置 PSP
    __set_PSP(first_sp);
    // 切换到 PSP 模式
    __set_CONTROL(0x00);
    __set_CONTROL(0x2);
    __ISB();
    // 触发 PendSV 开始调度
    HAL_TIM_Base_Start_IT(&htim1);
    //进入0号任务
    ELVNRTOS_IdleTask(NULL);
}

//删除任务
TaskControlBlock * task_delete;
void ELVNRTOS_Delete(TaskControlBlock *task){
    ENTER_CRITICAL();
    if(task==NULL) task_delete=Task_Current_TCB;
    else task_delete=task;
    EXIT_CRITICAL();
    ETask_Yield();
}

//系统初始化
uint8_t ELVNRTOS_Init(void){
    HAL_NVIC_SetPriority(PendSV_IRQn,15,0);
    NVIC_SetPriority(SysTick_IRQn, 0xF0);  // 比 PendSV 高
    //创建0号任务
    if(ETask_Creat(ELVNRTOS_IdleTask,0,256)==0)
        return 0;
    Task_Current_TCB=ready_head[0];
    return 1;
}

//调度器
void ETask_Scheduler(void){
    TaskControlBlock *task;
    TaskControlBlock *reg;
    if (task_delete != NULL) {
        if (NodePool_Release(task_delete) == 1) {
            Heap_Release((uint8_t *)task_delete->stack_base,task_delete->size);
        }
        task_delete = NULL;
    }
    for(int i=0;i<PRIORITY_COUNT;i++){
        if(ready_head[i]){
            task=ready_head[i];
            for (task = ready_head[i]; task != NULL; task = task->next){
                if(task->next==0&&task!=ready_head[i]) break;
                if(task->state!=TASK_STATE_READY&&task->state!=TASK_STATE_SUSPENDED)task->state--;
            }
        }
    }
    for(int i=PRIORITY_COUNT-1;i>=0;i--){
        if(ready_head[i]){
            task=ready_head[i];
            for (task = ready_head[i]; task != NULL; task = task->next){
                if(task->state==TASK_STATE_READY){
                    Task_Current_TCB=task;
                    if(task==ready_head[i]){
                        task->prev=ready_tail[i];
                        ready_tail[i]->next=task;
                        ready_head[i]=task->next;
                        ready_tail[i]=task;
                        task->next=0;
                        ready_head[i]->prev=0;
                    }
                    else if (task != ready_tail[i]) {
                        reg = task->prev;
                        reg->next = task->next;       /* 前驱跳过 task */
                        task->next->prev = reg;       /* ← 补这句：后继的 prev 指回前驱 */
                        ready_tail[i]->next = task;   /* 挂到队尾 */
                        task->prev = ready_tail[i];
                        task->next = 0;
                        ready_tail[i] = task;
                    }
                    TIM1->CR1 |= TIM_CR1_CEN;
                    return;
                }
            }
        }
    }
}

void ETask_Yield(void){
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void Etask_Delay(uint32_t num){
    if (Task_Current_TCB != NULL){
        ENTER_CRITICAL();
        Task_Current_TCB->state=num;
        EXIT_CRITICAL();
        ETask_Yield();
    }
}

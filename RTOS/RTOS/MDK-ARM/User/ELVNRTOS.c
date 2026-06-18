#include "ELVNRTOS.h"
#include "OLED.h"

volatile uint8_t Task_Count=0;//当前任务数量
volatile uint8_t Task_Priority_Max=0;//当前最高优先级
volatile uint8_t Task_Current=0;//当前运行任务编号+1
volatile Task_Sign * Task_Current_TCB=0;
volatile Task_Sign Task_Sign_Table[10]={0};
__attribute__((__align(4))) uint32_t Task_Heap[HEAP_SIZE_FOR_PILE][PILE_SIZE]={0};
HEAP_STATE Heap_State[HEAP_SIZE_FOR_PILE]={IDLE,IDLE,IDLE};

uint32_t * Heap_Insert(uint16_t size)
{
    int num=(size+PILE_SIZE-1)/PILE_SIZE;
    int count=0;
    uint32_t * addr=0;
    for(int i=0;i<HEAP_SIZE_FOR_PILE;i++)
    {
        if(Heap_State[i]==BUSY)
        {
            count=0;
        }
        else if(Heap_State[i]==IDLE)
        {
            if(count==0)
            {
                addr=(uint32_t *)&Task_Heap[i][0];
                //return (uint32_t *)Task_Heap[0];
            }
            count++;
            if(count==num)
            {
                for(int j=0;j<num;j++)
                {
                    Heap_State[i-j]=BUSY;
                }
                return addr;
            }
        }
    }
    return 0;
}

int dsb=0;

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
        dsb++;
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

void test_A(void)
{
    OLED_ShowNum(1,1,__get_BASEPRI(),10);
    OLED_ShowNum(2,1,(uint32_t)Task_Sign_Table[0].SP,10);
    OLED_ShowNum(3,1,__get_PSP(),10);
    OLED_ShowNum(4,1,__HAL_TIM_GET_COUNTER(&htim1),10);
}

void test_B(void)
{
    test_A();
    OLED_ShowNum(1,1,__get_BASEPRI(),10);
    OLED_ShowNum(2,1,(uint32_t)Task_Sign_Table[0].SP,10);
    OLED_ShowNum(3,1,__get_PSP(),10);
    OLED_ShowNum(4,1,__HAL_TIM_GET_COUNTER(&htim1),10);
}

//空闲任务
task_retrun ELVNRTOS_IdleTask(task_parameter a)
{

    while(1)
    {
        // OLED_ShowNum(1,1,(uint32_t)Task_Sign_Table[0].SP,10);
        // OLED_ShowNum(2,1,(uint32_t)Task_Sign_Table[1].SP,10);
        // OLED_ShowNum(3,1,(uint32_t)Task_Sign_Table[2].SP,10);
        // OLED_ShowNum(4,1,(uint32_t)Task_Sign_Table[3].SP,10);
        test_B();
        OLED_Clear();
        //__WFI();
    }
}

uint8_t ETask_Creat(Task_InitStruct * a)
{
    //if((Task_Heap_Offset+a->Size)>Heap_MAX||Task_Count >= MAX_TASK_NUM||Task_Count>=MAX_TASK_NUM)return 0;
    // 获取栈空间
    // uint32_t *stack_top =HETask_Sign_Table[Task_Count].BP= Task_Heap+Task_Heap_Offset+a->Size;

    uint32_t *stack_top =Heap_Insert(a->Size)+a->Size;
    if (stack_top==0)
    {
        return 0;
    }
    uint32_t *sp = stack_top;
    // ========== 关键：正确的栈帧顺序 ==========
    // CPU 异常返回时，期望的栈顺序（从高到低）：
    // [xPSR] [PC] [LR] [R12] [R3] [R2] [R1] [R0] 硬件栈
    // [R4] [R5] [R6] [R7] [R8] [R9] [R10] [R11] 软件栈
    
    *--sp = 0x01000000;              // xPSR (Thumb位=1)
    *--sp = (uint32_t)a->Fuction;    // PC (任务入口)
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
    Task_Sign_Table[Task_Count].SP = sp;      // 指向 R0
    Task_Sign_Table[Task_Count].State = 0;
    Task_Sign_Table[Task_Count].Priority = a->Priority;
    Task_Sign_Table[Task_Count].SP_MAX=stack_top-a->Size;
    //任务数加1
    Task_Count++;
    //创建成功
    return 1;
}

void ETask_StartScheduler(void)
{
    if(Task_Count == 0) return;
    // 获取第一个任务的 sp（指向 R0）
    uint32_t first_sp = (uint32_t)Task_Sign_Table[0].SP;
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
uint8_t ELVNRTOS_Delete(uint8_t Task_Current)
{
    if(Task_Current < Task_Count)
    {

    }
}

//系统初始化
uint8_t ELVNRTOS_Init(void)
{
    HAL_NVIC_SetPriority(PendSV_IRQn,15,0);
    NVIC_SetPriority(SysTick_IRQn, 0xF0);  // 比 PendSV 高
    Task_InitStruct ELVNRTOS_IdleTask_Init = {
        .Handle     = 0,  // 直接用静态数组
        .Fuction    = ELVNRTOS_IdleTask,
        .Size       = 128,
        .Priority = 0
    };
    //创建0号任务
    if(ETask_Creat(&ELVNRTOS_IdleTask_Init)==0)
        return 0;
    Task_Current_TCB=&Task_Sign_Table[0];
    return 1;
}

//调度器
void ETask_Scheduler(void)
{
    uint8_t Task_Current1=0;
    Task_Priority_Max=0;
    for(int i=0;i<Task_Count;i++)
    {
        if(Task_Sign_Table[(Task_Current+i+1)%Task_Count].State==0&&Task_Sign_Table[(Task_Current+i+1)%Task_Count].Priority>Task_Priority_Max)
        {
            Task_Current1=(Task_Current+i+1)%Task_Count;
            Task_Priority_Max=Task_Sign_Table[(Task_Current+i+1)%Task_Count].Priority;
        }
        if(Task_Sign_Table[(Task_Current+i+1)%Task_Count].State&&Task_Sign_Table[(Task_Current+i+1)%Task_Count].State!=ERTOS_Delay_MAx)
        {
            Task_Sign_Table[(Task_Current+i+1)%Task_Count].State--;
        }
    }
    Task_Current=Task_Current1;
    Task_Current_TCB=&Task_Sign_Table[Task_Current];
    TIM1->CR1 |= TIM_CR1_CEN;
}

void ETask_Yield(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void Etask_Delay(uint32_t num)
{
    Task_Sign_Table[Task_Current].State=num;
    //SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    ETask_Yield();
}

#include "ELVNRTOS.h"

volatile uint8_t Task_Count=0;//当前任务数量
volatile uint8_t Task_Priority_Max=0;//当前最高优先级
volatile uint8_t Task_Current=0;//当前运行任务编号+1

Task_Sign Task_Sign_Table[MAX_TASK_NUM]={0};//任务标志表
handle ELVNRTOS_IdleTask_Handle;//空闲任务句柄
Task_InitStruct ELVNRTOS_IdleTask_Init={&ELVNRTOS_IdleTask_Handle,ELVNRTOS_IdleTask,"IdleTask",128};//空闲任务初始化结构体

//时间片轮转
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Trigger) != RESET)	//检查TIM2触发中断发生与否
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Trigger);	//清除TIM2触发中断标志
        ELVNRTOS_Scheduler();
	}
}

//空闲任务
void ELVNRTOS_IdleTask(void)
{
    while(1)
    {
        __WFI();
    }
}

//创建任务
uint8_t ELVNRTOS_Creat(Task_InitStruct * a)
{
    if(Task_Count>=MAX_TASK_NUM)
    {
        return 0;
    }
    *(a->Handle)=malloc(a->Size*4);
    if(*(a->Handle)==NULL)
    {
        return 0;
    }
    memset(*(a->Handle),0,a->Size*4);
    Task_Sign_Table[Task_Count].Task_SP_MAX=*(a->Handle);
    //Task_Sign_Table[Task_Count].Task_BP=*(a->Handle)+a->Size;
    Task_Sign_Table[Task_Count].Task_SP=Task_Sign_Table[Task_Count].Task_BP-7;
    if(a->Task_Priority>TASK_PRIORITY_MAX)
    {
        Task_Sign_Table[Task_Count].Task_Priority=TASK_PRIORITY_MAX;
    }
    Task_Count++;
    
    return 1;
}

//删除任务
uint8_t ELVNRTOS_Delete(uint8_t Task_Current)
{
    if(Task_Current < Task_Count)
    {

    }

}

//系统初始化
uint8_t ELVNRTOS_Init(Task_InitStruct * a)
{
    ELVNRTOS_Creat(&ELVNRTOS_IdleTask_Init);
}

uint8_t ELVNRTOS_EnterRTOS(Task_InitStruct * a)
{
    TIM_Cmd(TIM2, ENABLE);	//使能TIM2
}

//调度器
uint8_t ELVNRTOS_Scheduler(void)
{
    Task_Sign_Table[Task_Current].Task_SP=(uint32_t *)__get_PSP();
    uint8_t Task_Current1=0;
    for(int i=0;i<Task_Count;i++)
    {
        if(Task_Sign_Table[(Task_Current+i)%Task_Count].Task_State==0||Task_Sign_Table[(Task_Current+i)%Task_Count].Task_Priority>Task_Priority_Max)
        {
            Task_Current1=(Task_Current+i)%Task_Count;
            Task_Priority_Max=Task_Sign_Table[(Task_Current+i)%Task_Count].Task_Priority;
        }
        if(Task_Sign_Table[(Task_Current+i)%Task_Count].Task_State)
        {
            Task_Sign_Table[(Task_Current+i)%Task_Count].Task_State--;
        }
    }
    Task_Current=Task_Current1;
    __set_PSP((uint32_t)Task_Sign_Table[Task_Current].Task_SP);
}


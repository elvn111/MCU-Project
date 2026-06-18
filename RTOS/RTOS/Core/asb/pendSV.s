; port.s
    AREA    |.text|, CODE, READONLY
    THUMB
    EXPORT  PendSV_Handler
    IMPORT  Task_Current
    IMPORT  Task_Sign_Table
    IMPORT  Task_Count

PendSV_Handler
    ; 保存当前任务现场
    MRS     R0, PSP              ; R0 = 当前任务的 sp（指向 R0）
    STMDB   R0!, {R4-R11}        ; 保存 R4-R11，R0 递减
    MSR     PSP, R0              ; 更新 PSP
    
    ; 保存当前任务的 sp
    LDR     R1, =Task_Current
    LDRB    R1, [R1]
    LDR     R2, =Task_Sign_Table
    LSL     R1, R1, #2          ; 假设 TaskCB_t 大小 4 字节
    ADD     R2, R2, R1
    STR     R0, [R2]             ; 保存到 Task_Sign_Table[idx].sp
    
    ; 切换任务（简单轮转）
    LDR     R1, =Task_Current
    LDRB    R3, [R1]
    ADD     R3, R3, #1
    LDR     R4, =Task_Count
    LDRB    R4, [R4]
    CMP     R3, R4
    IT      GE
    MOVGE   R3, #0
    STRB    R3, [R1]             ; Task_Current= next
    
    ; 加载新任务的 sp
    LSL     R3, R3, #2
    ADD     R2, R2, R3
    LDR     R0, [R2]             ; R0 = 新任务的 sp（指向 R0）
    
    ; 恢复新任务现场
    LDMIA   R0, {R4-R11}         ; 恢复 R4-R11
    ADDS    R0, R0, #32
    MSR     PSP, R0              ; 设置新 PSP（指向 R0）
    
    BX      LR
    
    ALIGN
    END
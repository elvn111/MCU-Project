#ifndef __KEY_H__
#define __KEY_H__

// 按键位定义
#define KEY_CONFIRM  1 // bit0: 确认
#define KEY_UP       2  // bit1: 上
#define KEY_DOWN     4  // bit2: 下
#define KEY_EXIT     16  // bit3: 退出或复位
#define KEY_MIDLE    8 //bit4
#define KEY_A        32 //bit5

void ReadKeyTask (void);

#endif

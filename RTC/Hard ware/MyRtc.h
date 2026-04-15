#ifndef __MYRTC_H__
#define __MYRTC_H__

void MyRtc_SetTime(void);
void MyRtc_Init(void);
void MyRtc_ReadTime(void);

extern uint16_t MyTime[6];

#endif
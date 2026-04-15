#ifndef __IIC_H__
#define __IIC_H__

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_Send_byte(unsigned char txd);
unsigned char IIC_Wait_byte(void);
void IIC_Send_ACK(unsigned char ack);
unsigned char IIC_Wait_ACK(void);

#endif

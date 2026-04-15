#include "stm32f10x.h"                  // Device header

#define IIC_SCL_PIN GPIO_Pin_10
#define IIC_SDA_PIN GPIO_Pin_11
#define SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)(x))
#define SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)(x))
//sbit SDA = P2^0; //定义IIC数据线
//sbit SCL = P2^1; //定义IIC时钟线


void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initstructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能GPIOB时钟
	GPIO_Initstructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Initstructure.GPIO_Pin= GPIO_Pin_10;
	GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Initstructure);
	
	GPIO_Initstructure.GPIO_Mode= GPIO_Mode_Out_OD;
	GPIO_Initstructure.GPIO_Pin= GPIO_Pin_11;
	GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_Initstructure);
}

void IIC_Start(void)
{
    SDA(1);
    SCL(1);
    SDA(0);
    SCL(0);
}

void IIC_Stop(void)
{
    SDA(0);
    SCL(1);
    SDA(1);
}

void IIC_Send_byte(unsigned char txd)
{
    unsigned char i;
    for(i=0; i<8; i++)
    {
        
        SDA(txd & (0x80>>i)); //发送最高位
        SCL(1); //时钟线拉高，数据被读
        SCL(0);
    }
    SCL(0); //时钟线拉低，准备接收ACK
}

unsigned char IIC_Wait_byte(void)
{
    unsigned char i, rxd = 0;
    SDA(1); //释放数据线，准备接收数据
    for(i=0; i<8; i++)
    {
        SCL(0); //时钟线拉低
        SCL(1); //时钟线拉高，准备读取数据

        rxd <<= 1; //左移一位
        if(GPIO_ReadInputDataBit(GPIOB, IIC_SDA_PIN)) //读取数据位
            rxd |= 0x01; //如果SDA为高电平，设置最低位为1
    }
    return rxd; //返回接收到的字节
}

void IIC_Send_ACK(unsigned char ack)
{
    SCL(0); //时钟线拉低
    SDA(ack); //发送ACK信号
    SCL(1); //时钟线拉高，ACK信号被读取
    SCL(0); //时钟线拉低，准备下一次通信
}

unsigned char IIC_Wait_ACK(void)
{
    unsigned char ack;
    SCL(0); //时钟线拉低
    SCL(1); //时钟线拉高，准备读取ACK
    ack = GPIO_ReadInputDataBit(GPIOB, IIC_SDA_PIN); //读取ACK信号
    SCL(0); //时钟线拉低，准备下一次通信
    return ack; //返回ACK信号
}

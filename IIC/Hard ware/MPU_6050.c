#include "IIC.h"

unsigned char MPU_6050_Init(void)
{
    IIC_Init(); // 初始化I2C接口
    IIC_Start();
    IIC_Send_byte(0xD0); // 发送设备地址+写命令
    if (IIC_Wait_ACK()) {
        IIC_Stop();
        return 1; // 无应答，初始化失败
    }
    
    IIC_Send_byte(0x6B); // 发送寄存器地址（电源管理寄存器）
    if (IIC_Wait_ACK()) {
        IIC_Stop();
        return 1; // 无应答，初始化失败
    }
    
    IIC_Send_byte(0x00); // 发送数据（解除休眠模式）
    if (IIC_Wait_ACK()) {
        IIC_Stop();
        return 1; // 无应答，初始化失败
    }
    
    IIC_Stop(); // 停止I2C通信
    return 0; // 初始化成功
}

 char MPU_6050_ReadData(unsigned char RegAddr)
{
     char Data;
    
    IIC_Start();
    IIC_Send_byte(0xD0); // 发送设备地址+写命令
    if (IIC_Wait_ACK()) {
        IIC_Stop();
        return 0; // 无应答，读取失败
    }
    
    IIC_Send_byte(RegAddr); // 发送寄存器地址
    if (IIC_Wait_ACK()) {
        IIC_Stop();
        return 0; // 无应答，读取失败
    }
    
    IIC_Start(); // 重复起始条件
    IIC_Send_byte(0xD1); // 发送设备地址+读命令
    if (IIC_Wait_ACK()) {
        IIC_Stop();
        return 0; // 无应答，读取失败
    }
    
    Data = IIC_Wait_byte(); // 读取数据
    IIC_Send_ACK(1); // 发送ACK信号
    IIC_Stop(); // 停止I2C通信
    
    return Data; // 返回读取的数据
}


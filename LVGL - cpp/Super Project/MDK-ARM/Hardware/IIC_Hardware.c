/**
  ******************************************************************************
  * @file    IIC_Hardware.c
  * @brief   IIC硬件层驱动文件，用于MPU6050传感器通信
  * @author  Elvn
  * @version V1.0
  * @date    2026-03-26
  ******************************************************************************
  */

#include "stm32f1xx_hal.h"
#include "IIC_Hardware.h"
#include "i2c.h"
#include <stdio.h>

/**
  * @brief  IIC1初始化函数，用于初始化MPU6050传感器
  * @param  无
  * @retval 无
  * @note   唤醒MPU6050并读取设备地址进行验证
  */
void IIC1_Init(void)
{
    uint8_t ab;  // 存储设备地址的变量
    
    // 向MPU6050的PWR_MGMT_1寄存器(0x6B)写入0x00，唤醒传感器
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x6b, 0x00);
    
    // 打印初始化完成信息
    lprintf("[MPU]:Init Over\n");
    
    // 读取MPU6050的WHO_AM_I寄存器(0x75)，获取设备地址
    HAL_I2C_Mem_Read(&hi2c2, 0xd0, 0x75, I2C_MEMADD_SIZE_8BIT, &ab, 1, HAL_MAX_DELAY);
    
    // 打印设备地址信息
    lprintf("[MPU]:Devices Address is %d\n", ab);
    
    // 校准函数暂时注释，可根据需要取消注释
    // MPU6050_Calibration();
    // printf("[MPU]:Calibration Over \n");
}

/**
  * @brief  IIC写入一个字节数据
  * @param  addr: 设备地址
  * @param  reg: 寄存器地址
  * @param  data: 要写入的数据
  * @retval 无
  * @note   使用HAL库的I2C主发送函数实现
  */
void IIC1_WriteByte(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t total[2] = {reg, data};  // 组合寄存器地址和数据
    
    // 使用HAL库函数发送数据
    HAL_I2C_Master_Transmit(&hi2c2, addr, total, 2, HAL_MAX_DELAY);
}

/**
  * @brief  IIC读取一个字节数据
  * @param  addr: 设备地址
  * @param  reg: 寄存器地址
  * @retval 读取到的数据
  * @note   使用HAL库的I2C内存读取函数实现
  */
uint8_t IIC1_ReadByte(uint8_t addr, uint8_t reg)
{
    uint8_t data;  // 存储读取的数据
    
    // 使用HAL库函数读取数据
    HAL_I2C_Mem_Read(&hi2c2, addr, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
    
    return data;  // 返回读取到的数据
}

/**
  * @brief  MPU6050传感器校准函数
  * @param  无
  * @retval 无
  * @note   通过多次采样计算加速度计和陀螺仪的偏移值，并写入偏移寄存器
  */
void MPU6050_Calibration(void)
{
    int32_t ax_sum = 0, ay_sum = 0, az_sum = 0;  // 加速度计各轴数据总和
    int32_t gx_sum = 0, gy_sum = 0, gz_sum = 0;  // 陀螺仪各轴数据总和
    uint8_t buf[14];  // 存储读取的数据
    int i;  // 循环变量

    // 先唤醒MPU6050并复位偏移
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x6B, 0x00);  // 唤醒传感器
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x1B, 0x00);  // 陀螺仪量程设置为±250°/s
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x1C, 0x00);  // 加速度计量程设置为±2g
    HAL_Delay(100);  // 延时100ms，确保传感器稳定

    // 读取1000次数据求平均值
    for(i = 0; i < 1000; i++)
    {
        // 读取MPU6050的加速度计和陀螺仪数据（从0x3B寄存器开始，共14个字节）
        HAL_I2C_Mem_Read(&hi2c2, MPU_6050_DEVICES_ADDRESS, 0x3B, I2C_MEMADD_SIZE_8BIT, buf, 14, HAL_MAX_DELAY);
        
        // 计算加速度计各轴数据总和（16位有符号数）
        ax_sum += (buf[0] << 8) | buf[1];  // X轴加速度
        ay_sum += (buf[2] << 8) | buf[3];  // Y轴加速度
        az_sum += (buf[4] << 8) | buf[5];  // Z轴加速度
        
        // 计算陀螺仪各轴数据总和（16位有符号数）
        gx_sum += (buf[8] << 8) | buf[9];  // X轴角速度
        gy_sum += (buf[10] << 8) | buf[11];  // Y轴角速度
        gz_sum += (buf[12] << 8) | buf[13];  // Z轴角速度
        
        HAL_Delay(2);  // 延时2ms，控制采样频率
    }

    // 计算平均值并取反，得到偏移值
    int16_t ax_off = -(ax_sum / 1000);  // X轴加速度偏移
    int16_t ay_off = -(ay_sum / 1000);  // Y轴加速度偏移
    int16_t az_off = -(az_sum / 1000) + 16384;  // Z轴加速度偏移，需要加上1g（16384）
    int16_t gx_off = -(gx_sum / 1000);  // X轴陀螺仪偏移
    int16_t gy_off = -(gy_sum / 1000);  // Y轴陀螺仪偏移
    int16_t gz_off = -(gz_sum / 1000);  // Z轴陀螺仪偏移

    // 写入加速度计偏移寄存器（每个轴占用两个寄存器）
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_X_OFFSET, ax_off >> 8);  // X轴加速度偏移高8位
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_X_OFFSET+1, ax_off & 0xFF);  // X轴加速度偏移低8位

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Y_OFFSET, ay_off >> 8);  // Y轴加速度偏移高8位
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Y_OFFSET+1, ay_off & 0xFF);  // Y轴加速度偏移低8位

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Z_OFFSET, az_off >> 8);  // Z轴加速度偏移高8位
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Z_OFFSET+1, az_off & 0xFF);  // Z轴加速度偏移低8位

    // 写入陀螺仪偏移寄存器（每个轴占用两个寄存器）
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_X_OFFSET_H, gx_off >> 8);  // X轴陀螺仪偏移高8位
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_X_OFFSET_L, gx_off & 0xFF);  // X轴陀螺仪偏移低8位

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Y_OFFSET_H, gy_off >> 8);  // Y轴陀螺仪偏移高8位
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Y_OFFSET_L, gy_off & 0xFF);  // Y轴陀螺仪偏移低8位

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Z_OFFSET_H, gz_off >> 8);  // Z轴陀螺仪偏移高8位
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Z_OFFSET_L, gz_off & 0xFF);  // Z轴陀螺仪偏移低8位
}

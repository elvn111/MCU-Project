#include "stm32f1xx_hal.h"
#include "IIC_Hardware.h"
#include "i2c.h"
#include <stdio.h>
void IIC1_Init(void)
{
    uint8_t ab;
	IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS,0x6b,0x00);
    printf("[MPU]:Init Over \n");
    HAL_I2C_Mem_Read(&hi2c1,0xd0,0x75,I2C_MEMADD_SIZE_8BIT,&ab,1,HAL_MAX_DELAY);
    printf("[MPU]:Devices Address is %d \n",ab);
    // MPU6050_Calibration();
    // printf("[MPU]:Calibration Over \n");

}

void IIC1_WriteByte(uint8_t addr,uint8_t reg,uint8_t data)
{
    uint8_t total[2]={reg,data};
    HAL_I2C_Master_Transmit(&hi2c1,addr,total,2,HAL_MAX_DELAY);
}

uint8_t IIC1_ReadByte(uint8_t addr,uint8_t reg)
{
    uint8_t data;
    HAL_I2C_Mem_Read(&hi2c1,addr,reg,I2C_MEMADD_SIZE_8BIT,&data,1,HAL_MAX_DELAY);

    return data;
    
}

void MPU6050_Calibration(void)
{
    int32_t ax_sum = 0, ay_sum = 0, az_sum = 0;
    int32_t gx_sum = 0, gy_sum = 0, gz_sum = 0;
    uint8_t buf[14];
    int i;

    // 先唤醒 + 复位偏移
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x6B, 0x00);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x1B, 0x00);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, 0x1C, 0x00);
    HAL_Delay(100);

    // 读取1000次求平均
    for(i = 0; i < 1000; i++)
    {
       // IIC_ReadLen(MPU_6050_DEVICES_ADDRESS, 0x3B, 14, buf);
        HAL_I2C_Mem_Read(&hi2c1,MPU_6050_DEVICES_ADDRESS,0x3B,I2C_MEMADD_SIZE_8BIT,buf,14,HAL_MAX_DELAY);
        ax_sum += (buf[0] << 8) | buf[1];
        ay_sum += (buf[2] << 8) | buf[3];
        az_sum += (buf[4] << 8) | buf[5];
        gx_sum += (buf[8] << 8) | buf[9];
        gy_sum += (buf[10] << 8) | buf[11];
        gz_sum += (buf[12] << 8) | buf[13];
        HAL_Delay(2);
    }

    // 计算平均值
    int16_t ax_off = -(ax_sum / 1000);
    int16_t ay_off = -(ay_sum / 1000);
    int16_t az_off = -(az_sum / 1000) + 16384; // Z轴要减掉1g
    int16_t gx_off = -(gx_sum / 1000);
    int16_t gy_off = -(gy_sum / 1000);
    int16_t gz_off = -(gz_sum / 1000);

    // 写入偏移寄存器
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_X_OFFSET, ax_off >> 8);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_X_OFFSET+1, ax_off & 0xFF);

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Y_OFFSET, ay_off >> 8);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Y_OFFSET+1, ay_off & 0xFF);

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Z_OFFSET, az_off >> 8);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_ACCEL_Z_OFFSET+1, az_off & 0xFF);

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_X_OFFSET_H, gx_off >> 8);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_X_OFFSET_L, gx_off & 0xFF);

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Y_OFFSET_H, gy_off >> 8);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Y_OFFSET_L, gy_off & 0xFF);

    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Z_OFFSET_H, gz_off >> 8);
    IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS, MPU_6050_GYRO_Z_OFFSET_L, gz_off & 0xFF);
}

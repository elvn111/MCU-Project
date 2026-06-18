#ifndef __IIC_HARDWARE_H__
#define __IIC_HARDWARE_H__

#define MPU_6050_DEVICES_ADDRESS 0xD0
#define MPU_6050_ACCEL_XOUT_H 0x3B
#define MPU_6050_ACCEL_XOUT_L 0x3C
#define MPU_6050_ACCEL_YOUT_H 0x3D
#define MPU_6050_ACCEL_YOUT_L 0x3E
#define MPU_6050_ACCEL_ZOUT_H 0x3F
#define MPU_6050_ACCEL_ZOUT_L 0x40


// 加速度矫正偏移寄存器
#define MPU_6050_ACCEL_X_OFFSET 0x06
#define MPU_6050_ACCEL_Y_OFFSET 0x08
#define MPU_6050_ACCEL_Z_OFFSET 0x0A

// 陀螺仪 配置寄存器
#define MPU_6050_GYRO_XOUT_H        0x43    // X轴 高8位
#define MPU_6050_GYRO_XOUT_L        0x44    // X轴 低8位

#define MPU_6050_GYRO_YOUT_H        0x45    // Y轴 高8位
#define MPU_6050_GYRO_YOUT_L        0x46    // Y轴 低8位

#define MPU_6050_GYRO_ZOUT_H        0x47    // Z轴 高8位
#define MPU_6050_GYRO_ZOUT_L        0x48    // Z轴 低8位

// 陀螺仪 校准偏移寄存器
#define MPU_6050_GYRO_X_OFFSET_H    0x13
#define MPU_6050_GYRO_X_OFFSET_L    0x14

#define MPU_6050_GYRO_Y_OFFSET_H    0x15
#define MPU_6050_GYRO_Y_OFFSET_L    0x16

#define MPU_6050_GYRO_Z_OFFSET_H    0x17
#define MPU_6050_GYRO_Z_OFFSET_L    0x18

void IIC1_WriteByte(uint8_t addr,uint8_t reg,uint8_t data);
void IIC1_Init(void);
void MPU6050_Calibration(void);
uint8_t IIC1_ReadByte(uint8_t addr,uint8_t reg);

#endif

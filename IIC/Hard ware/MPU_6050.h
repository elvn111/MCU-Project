#ifndef __MPU_6050_H__
#define __MPU_6050_H__  

#define MPU_6050_ACCEL_XOUT_H 0x3B
#define MPU_6050_ACCEL_XOUT_L 0x3C
#define MPU_6050_ACCEL_YOUT_H 0x3D
#define MPU_6050_ACCEL_YOUT_L 0x3E
#define MPU_6050_ACCEL_ZOUT_H 0x3F
#define MPU_6050_ACCEL_ZOUT_L 0x40
unsigned char MPU_6050_ReadData(unsigned char RegAddr);
unsigned char MPU_6050_Init(void);

#endif

#ifndef __IIC_HARDWARE_H__
#define __IIC_HARDWARE_H__

#define MPU_6050_DEVICES_ADDRESS 0x68
#define MPU_6050_ACCEL_XOUT_H 0x3B
#define MPU_6050_ACCEL_XOUT_L 0x3C
#define MPU_6050_ACCEL_YOUT_H 0x3D
#define MPU_6050_ACCEL_YOUT_L 0x3E
#define MPU_6050_ACCEL_ZOUT_H 0x3F
#define MPU_6050_ACCEL_ZOUT_L 0x40

void IIC1_WriteByte(uint8_t addr,uint8_t reg,uint8_t data);
void IIC1_Init(void);
uint8_t IIC1_ReadByte(uint8_t addr,uint8_t reg);
void IIC_AMD_Init(uint32_t *pryaddr,uint32_t *memaddr,uint32_t n);
#endif
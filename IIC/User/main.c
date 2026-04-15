#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MPU_6050.h"

typedef union
{
    struct {
        uint8_t Lbyte;  // 低字节（对应HalfWord的0~7位）
        uint8_t Hbyte;  // 高字节（对应HalfWord的8~15位）
    } Byte;             // 结构体成员：拆分后的高低字节
    uint16_t HalfWord;  // 16位整体数据
} dsb;


int main(void)
{
	dsb X;
	dsb Y;
	dsb Z;
	MPU_6050_Init();
	OLED_Init();
	OLED_ShowString(1,1,"Axis:");
	OLED_ShowString(2,1,"X:");
	OLED_ShowString(3,1,"Y:");
	OLED_ShowString(4,1,"Z:");
	while(1)
	{
		X.Byte.Lbyte=MPU_6050_ReadData(MPU_6050_ACCEL_XOUT_L);
		X.Byte.Hbyte=MPU_6050_ReadData(MPU_6050_ACCEL_XOUT_H);
		Z.Byte.Lbyte=MPU_6050_ReadData(MPU_6050_ACCEL_ZOUT_L);
		Z.Byte.Hbyte=MPU_6050_ReadData(MPU_6050_ACCEL_ZOUT_H);
		Y.Byte.Lbyte=MPU_6050_ReadData(MPU_6050_ACCEL_YOUT_L);
		Y.Byte.Hbyte=MPU_6050_ReadData(MPU_6050_ACCEL_YOUT_H);
		OLED_ShowNum(2,3,(uint16_t)(X.HalfWord+32768),6);
		OLED_ShowNum(3,3,(uint16_t)(Y.HalfWord+32768),6);
		OLED_ShowNum(4,3,(uint16_t)(Z.HalfWord+32768),6);
	}
}
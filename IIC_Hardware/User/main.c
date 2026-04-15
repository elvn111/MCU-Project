#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "IIC_Hardware.h"

typedef union{
	struct 
	{
		uint8_t Lbyte;
		uint8_t Hbyte;
	}unite;
	int16_t total;
}Halfworld;

int main(void)
{
	Halfworld X;
	Halfworld Y;
	Halfworld Z;
	int slaveaddr;
	
	OLED_Init();
	IIC1_Init();
//	OLED_ShowString(1,1,"Axis");
//	OLED_ShowString(2,1,"X:");
//	OLED_ShowString(3,1,"Y:");
//	OLED_ShowString(4,1,"Z:");
	while (1)
	{
		X.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_XOUT_L);
		X.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_XOUT_H);
		Y.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_YOUT_L);
		Y.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_YOUT_H);
//		Z.unite.Lbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_ZOUT_L);
//		Z.unite.Hbyte=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,MPU_6050_ACCEL_ZOUT_H);
		slaveaddr=IIC1_ReadByte(MPU_6050_DEVICES_ADDRESS,0x75);
		OLED_Horizon((int16_t)X.total,(int16_t)Y.total);
//		OLED_ShowNum(4,3,slaveaddr,6);
//		OLED_ShowNum(2,3,X.total,6);
//		OLED_ShowNum(3,3,Y.total,6);
		//OLED_ShowNum(4,3,Z.total,6);
	}
	
}
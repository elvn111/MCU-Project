#include "stm32f10x.h"
#include "IIC_Hardware.h"
void IIC1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);

    GPIO_InitTypeDef GPIO_Initstructure;
    GPIO_Initstructure.GPIO_Pin =GPIO_Pin_6|GPIO_Pin_7;
    GPIO_Initstructure.GPIO_Mode=GPIO_Mode_AF_OD;
    GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_Initstructure);

    I2C_InitTypeDef I2C_Initstructure;
    I2C_Initstructure.I2C_Ack=I2C_Ack_Enable;
    I2C_Initstructure.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;
    I2C_Initstructure.I2C_ClockSpeed=400000;
    I2C_Initstructure.I2C_DutyCycle=I2C_DutyCycle_2;
    I2C_Initstructure.I2C_Mode=I2C_Mode_I2C;
    I2C_Initstructure.I2C_OwnAddress1=0x00;
    I2C_Init(I2C1,&I2C_Initstructure);

    I2C_Cmd(I2C1,ENABLE);
		IIC1_WriteByte(MPU_6050_DEVICES_ADDRESS,0x6b,0x00);
    //I2C_ITConfig(I2C1,i2c_it_)

}

void IIC1_WriteByte(uint8_t addr,uint8_t reg,uint8_t data)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1,addr<<1,I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1,reg);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(I2C1,data);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C1,ENABLE);
}

uint8_t IIC1_ReadByte(uint8_t addr,uint8_t reg)
{
    uint8_t data;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1,addr<<1,I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1,reg);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1,addr<<1,I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    I2C_AcknowledgeConfig(I2C1, DISABLE); // 单字节接收，禁用ACK
    I2C_GenerateSTOP(I2C1, ENABLE); // 发送停止信号

    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)); // 等待接收完成
    data = I2C_ReceiveData(I2C1);

    // 恢复ACK使能
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return data;
    
}
 
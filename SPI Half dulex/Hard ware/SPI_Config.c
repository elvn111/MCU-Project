#include "stm32f10x.h"
#include "SPI_Config.h"
#include "OLED.h"

void SPI1_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO,ENABLE);
   
    GPIO_InitTypeDef GPIO_Initstructure;
    GPIO_Initstructure.GPIO_Mode=GPIO_Mode_AF_PP;
    GPIO_Initstructure.GPIO_Pin=GPIO_Pin_5|GPIO_Pin_7;
    GPIO_Initstructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_Initstructure);
    GPIO_Initstructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_Initstructure.GPIO_Pin=GPIO_Pin_6;
    GPIO_Init(GPIOA,&GPIO_Initstructure);
    GPIO_Initstructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Initstructure.GPIO_Pin=GPIO_Pin_4;
    GPIO_Init(GPIOA,&GPIO_Initstructure);

    SPI_InitTypeDef SPI_Initstructure;
    SPI_Initstructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_128;
    SPI_Initstructure.SPI_CPHA=SPI_CPHA_1Edge;
    SPI_Initstructure.SPI_CPOL=SPI_CPOL_Low;
    SPI_Initstructure.SPI_CRCPolynomial=7;
    SPI_Initstructure.SPI_DataSize=SPI_DataSize_8b;
    SPI_Initstructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
    SPI_Initstructure.SPI_FirstBit=SPI_FirstBit_MSB;
    SPI_Initstructure.SPI_Mode=SPI_Mode_Master;
    SPI_Initstructure.SPI_NSS=SPI_NSS_Soft;
    SPI_Init(SPI1,&SPI_Initstructure);

    SPI_Cmd(SPI1,ENABLE);

}
 
/**
 * @brief  SPI1单字节读写（全双工）
 * @param  tx_data: 要发送的字节
 * @retval 接收回来的字节
 * @note   核心底层函数，所有SPI指令/数据传输都依赖它
 */
uint8_t SPI1_ReadWriteByte(uint8_t tx_data)
{
    // 1. 等待发送缓冲区为空（TXE位：发送寄存器空）
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    
    // 2. 发送1个字节到SPI数据寄存器
    SPI_I2S_SendData(SPI1, tx_data);
    
    // 3. 等待接收缓冲区非空（RXNE位：接收寄存器有数据）
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    
    // 4. 返回接收到的字节
    return SPI_I2S_ReceiveData(SPI1);
}

void SPI1_RemoveWriteProtect(void)
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    SPI1_ReadWriteByte(W25Q64_CMD_WRITE_ENABLE);
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

uint8_t SPI1_ReadReg1(void)
{
    uint8_t data;
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    data=SPI1_ReadWriteByte(W25Q64_CMD_READ_STATUS_REG1);
    SPI1_ReadWriteByte(0XFF);
    //GPIO_SetBits(GPIOA,GPIO_Pin_4);
    return data;
}

void SPI1_WaitReg1(void)
{
    SPI1_ReadReg1();
    while ((SPI1_ReadWriteByte(0xff) & 0x01) == 0x01);
    //OLED_ShowNum(1,9,1,1);
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void SPI1_ReadDate(uint32_t addr,uint8_t * reg,uint16_t len)
{
    int i;
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);

    SPI1_ReadWriteByte(W25Q64_CMD_READ_DATA);
    SPI1_ReadWriteByte((uint8_t)(addr>>16));
    SPI1_ReadWriteByte((uint8_t)(addr>>8));
    SPI1_ReadWriteByte((uint8_t)(addr));
    for(i=0;len!=0;len--)
    {
        reg[i++]=SPI1_ReadWriteByte(0xFF);
    }
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void SPI1_EreseRom(uint32_t addr)
{
    SPI1_RemoveWriteProtect();
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    SPI1_ReadWriteByte(W25Q64_CMD_SECTOR_ERASE);
    SPI1_ReadWriteByte((uint8_t)(addr>>16));
    SPI1_ReadWriteByte((uint8_t)(addr>>8));
    SPI1_ReadWriteByte((uint8_t)(addr));
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
    SPI1_WaitReg1();
    OLED_ShowNum(1,9,1,1);

}

void SPI1_WriteData(uint32_t addr,uint8_t *reg,uint16_t len)
{
    int n=0;
    SPI1_RemoveWriteProtect();
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    SPI1_ReadWriteByte(W25Q64_CMD_PAGE_PROGRAM);
    SPI1_ReadWriteByte((uint8_t)(addr>>16));
    SPI1_ReadWriteByte((uint8_t)(addr>>8));
    SPI1_ReadWriteByte((uint8_t)(addr));
    while(len--)
    {
        SPI1_ReadWriteByte(reg[n++]);
    }
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
    SPI1_WaitReg1();
    OLED_ShowNum(2,9,1,1);
}

uint32_t SPI1_ReadJEDECID(void)
{
    uint32_t id = 0;
    GPIO_ResetBits(GPIOA, GPIO_Pin_4); // 拉低CS
    
    SPI1_ReadWriteByte(0x9F); // 发送读JEDEC ID指令
    id |= (uint32_t)SPI1_ReadWriteByte(0xFF) << 16; // 制造商ID（0xEF）
    id |= (uint32_t)SPI1_ReadWriteByte(0xFF) << 8;  // 类型ID（0x40）
    id |= (uint32_t)SPI1_ReadWriteByte(0xFF);       // 容量ID（0x17）
    
    GPIO_SetBits(GPIOA, GPIO_Pin_4); // 拉高CS
    return id;
}

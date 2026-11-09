#ifndef __I2C_H
#define __I2C_H

#include "main.h"

/* 硬件 I2C2：PB10 = SCL, PB11 = SDA（F103 硬件 I2C 引脚）
 * 供电控制：PB0 = GND（低电平导通）, PB1 = VCC（高电平供电） */
#define I2C_GND_PORT   GPIOB
#define I2C_GND_PIN    GPIO_PIN_0
#define I2C_VCC_PORT   GPIOB
#define I2C_VCC_PIN    GPIO_PIN_1

#define I2C_DEV_ADDR   0x78   /* SSD1306 写地址（0x3C << 1） */

void I2C_Init(void);
HAL_StatusTypeDef I2C_Transmit(uint16_t dev_addr, uint8_t *buf, uint16_t len);
HAL_StatusTypeDef I2C_WriteDataBlock(uint8_t *data, uint16_t len);

#endif /* __I2C_H */

#include "I2C.h"

static I2C_HandleTypeDef hi2c2;

void I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* 供电控制：PB0=GND(低), PB1=VCC(高) */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_WritePin(I2C_GND_PORT, I2C_GND_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(I2C_VCC_PORT, I2C_VCC_PIN, GPIO_PIN_SET);
	GPIO_InitStruct.Pin   = I2C_GND_PIN | I2C_VCC_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* I2C2 引脚：PB10=SCL, PB11=SDA，复用开漏，外部 4.7k 上拉 */
	__HAL_RCC_I2C2_CLK_ENABLE();
	GPIO_InitStruct.Pin   = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStruct.Mode  = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* 标准模式 100kHz */
	hi2c2.Instance             = I2C2;
	hi2c2.Init.ClockSpeed      = 400000;
	hi2c2.Init.DutyCycle       = I2C_DUTYCYCLE_2;
	hi2c2.Init.OwnAddress1     = 0;
	hi2c2.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c2.Init.OwnAddress2     = 0;
	hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c2.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
		__HAL_RCC_I2C2_FORCE_RESET();
		__HAL_RCC_I2C2_RELEASE_RESET();
		HAL_I2C_Init(&hi2c2);
	}
}

/* F1 的 HAL I2C 有 BUSY 卡死 bug：状态不对就重新初始化再传 */
static void I2C2_Recover(void)
{
	/* F1 I2C 卡死要看硬件 BUSY 位（SR2.BUSY），只查 HAL 状态不够 */
	if (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY ||
	    (hi2c2.Instance->SR2 & I2C_SR2_BUSY) != 0) {
		HAL_I2C_DeInit(&hi2c2);
		__HAL_RCC_I2C2_FORCE_RESET();
		__HAL_RCC_I2C2_RELEASE_RESET();
		HAL_I2C_Init(&hi2c2);
	}
}

HAL_StatusTypeDef I2C_Transmit(uint16_t dev_addr, uint8_t *buf, uint16_t len)
{
	I2C2_Recover();
	return HAL_I2C_Master_Transmit(&hi2c2, dev_addr, buf, len, 50);
}

/* SSD1306 数据模式：0x40 作为寄存器地址，一次发整块数据 */
/* SSD1306 数据块：0x40 控制字节 + 数据，按 128 字节/块 Master_Transmit 发
 * F1 HAL 的 Mem_Write 大块传输不稳，拆小块最可靠 */
HAL_StatusTypeDef I2C_WriteDataBlock(uint8_t *data, uint16_t len)
{
	static uint8_t chunk[129];
	uint16_t sent = 0;
	HAL_StatusTypeDef st = HAL_OK;
	chunk[0] = 0x40;
	while (sent < len) {
		uint16_t n = (len - sent > 128) ? 128 : (len - sent);
		memcpy(chunk + 1, data + sent, n);
		I2C2_Recover();
		st = HAL_I2C_Master_Transmit(&hi2c2, I2C_DEV_ADDR, chunk, n + 1, 50);
		if (st != HAL_OK) return st;
		sent += n;
	}
	return st;
}

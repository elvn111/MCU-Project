#ifndef __SPI_CONFIG_H__
#define __SPI_CONFIG_H__

// ====================== 基础指令 ======================
#define W25Q64_CMD_WRITE_ENABLE         0x06    // 写使能（擦除/编程前必须执行）
#define W25Q64_CMD_WRITE_DISABLE        0x04    // 写禁用
#define W25Q64_CMD_READ_STATUS_REG1     0x05    // 读状态寄存器1（最常用，判断忙闲）
#define W25Q64_CMD_READ_STATUS_REG2     0x35    // 读状态寄存器2
#define W25Q64_CMD_WRITE_STATUS_REG     0x01    // 写状态寄存器
#define W25Q64_CMD_READ_DEVICE_ID       0x90    // 读设备ID（含制造商ID+器件ID）
#define W25Q64_CMD_READ_JEDEC_ID        0x9F    // 读JEDEC ID（更常用的设备ID）
#define W25Q64_CMD_POWER_DOWN           0xB9    // 掉电模式（低功耗）
#define W25Q64_CMD_RELEASE_POWER_DOWN   0xAB    // 退出掉电模式（同时读ID）

// ====================== 读数据指令 ======================
#define W25Q64_CMD_READ_DATA            0x03    // 标准读（低速，最常用）
#define W25Q64_CMD_FAST_READ            0x0B    // 快速读（需加1个dummy时钟，高速）
#define W25Q64_CMD_FAST_READ_DUAL_IO    0x3B    // 双线快速读（MOSI/MISO同时传）
#define W25Q64_CMD_FAST_READ_QUAD_IO    0x6B    // 四线快速读（仅支持QPI模式）

// ====================== 编程（写）指令 ======================
#define W25Q64_CMD_PAGE_PROGRAM         0x02    // 页编程（单次最多写256字节）
#define W25Q64_CMD_QUAD_PAGE_PROGRAM    0x32    // 四线页编程（仅支持QPI模式）

// ====================== 擦除指令 ======================
#define W25Q64_CMD_SECTOR_ERASE         0x20    // 扇区擦除（4KB，最常用）
#define W25Q64_CMD_BLOCK_ERASE_32K      0x52    // 32KB块擦除
#define W25Q64_CMD_BLOCK_ERASE_64K      0xD8    // 64KB块擦除
#define W25Q64_CMD_CHIP_ERASE           0xC7    // 整片擦除（耗时久，慎用）

// ====================== 其他指令 ======================
#define W25Q64_CMD_ERASE_SECURITY_REG   0x44    // 擦除安全寄存器
#define W25Q64_CMD_PROGRAM_SECURITY_REG 0x42    // 编程安全寄存器
#define W25Q64_CMD_READ_SECURITY_REG    0x48    // 读安全寄存器
#define W25Q64_CMD_ERASE_SUSPEND        0x75    // 擦除暂停
#define W25Q64_CMD_ERASE_RESUME         0x7A    // 擦除恢复
#define W25Q64_CMD_READ_SFDP_REG        0x5A    // 读SFDP寄存器
#define W25Q64_CMD_ENTER_QPI_MODE       0x38    // 进入QPI模式（四线SPI）
#define W25Q64_CMD_EXIT_QPI_MODE        0xFF    // 退出QPI模式

// ====================== 辅助宏定义（方便编程） ======================
#define W25Q64_PAGE_SIZE                256     // 页大小（单次页编程最大字节数）
#define W25Q64_SECTOR_SIZE              4096    // 扇区大小（4KB）
#define W25Q64_BLOCK_32K_SIZE           32768   // 32KB块大小
#define W25Q64_BLOCK_64K_SIZE           65536   // 64KB块大小
#define W25Q64_CHIP_SIZE                8388608 // 芯片总容量（8MB = 64Mbit）

#define W25Q64_STATUS_REG1_BUSY_BIT     0x01    // 状态寄存器1的忙闲位（1=忙，0=空闲）

void SPI1_Init(void);
uint8_t SPI1_ReadWriteByte(uint8_t tx_data);
void SPI1_ReadDate(uint32_t addr,uint8_t * reg,uint16_t len);
void SPI1_WriteData(uint32_t addr,uint8_t *reg,uint16_t len);
void SPI1_EreseRom(uint32_t addr);
void SPI1_WaitReg1(void);
uint32_t SPI1_ReadJEDECID(void);

#endif

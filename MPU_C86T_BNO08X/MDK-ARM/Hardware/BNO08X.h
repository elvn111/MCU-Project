/**
  ******************************************************************************
  * @file    BNO08X.h
  * @brief   BNO08X (BNO080 / BNO085 / BNO086) 九轴姿态传感器驱动
  *
  *          本驱动用于替换原工程中的 MPU6050，输出航向角/俯仰角/横滚角。
  *
  *          支持两种接法（由 BNO08X_MODE 宏选择，默认 I2C）：
  *          1) I2C 模式（默认）：
  *             沿用原 MPU6050 的硬件 I2C2：PB10 = SCL，PB11 = SDA，400kHz。
  *             软件实现 SHTP + SH-2 协议：等待模块启动 -> 发送 Set Feature
  *             使能“旋转矢量(0x05)”上报 -> 解析四元数 -> 换算欧拉角。
  *          2) UART-RVC 模式：
  *             占用 USART3：PB10 = TX（本工程不用发，可不接），PB11 = RX，
  *             115200 8N1。模块需在上电时把 PS0 接 3V3 进入 RVC 模式，
  *             模块会以 100Hz 主动输出 19 字节固定帧（yaw/pitch/roll + 加速度）。
  *             本模式不需要协议交互，最简单，建议优先使用。
  *
  *          输出数据：
  *             yaw   : 航向角 0~360°，0° = 磁北（方向需磁力计校准）
  *             pitch : 俯仰角 -90~+90°
  *             roll  : 横滚角 -180~+180°
  *
  * @note    1. 源码是 STM32 HAL 库 + FreeRTOS 工程的一部分，
  *             BNO08X_Service() 需要在任务中频繁调用（建议 1~5ms 一次）。
  *          2. I2C 模式下模块的 INT 引脚可以不接（驱动采用读 SHTP 包头轮询的方式），
  *             如果模块 INT 已接到 MCU，可以把 BNO08X_INT_ENABLE 设为 1 降低 I2C 流量。
  *          3. RVC 模式的模块在 I2C 模式下不能同时使用，两种模式互斥。
  ******************************************************************************
  */
#ifndef __BNO08X_H__
#define __BNO08X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include <stdint.h>

/* ============================== 用户配置区 ============================== */

/* 工作模式：二选一 */
#define BNO08X_MODE_I2C        1   /* 硬件 I2C2 + SH-2 协议        */
#define BNO08X_MODE_UART_RVC   2   /* USART3 + UART-RVC 固定帧      */

#ifndef BNO08X_MODE
#define BNO08X_MODE            BNO08X_MODE_I2C
#endif

/* ---------------- I2C 模式参数 ---------------- */
/* 7 位从机地址：模块 AD0/SA0 接地 = 0x4A（多数模块默认），接高电平 = 0x4B */
#ifndef BNO08X_I2C_ADDR
#define BNO08X_I2C_ADDR        0x4A
#endif

/* 旋转矢量上报周期，单位微秒：10000 = 100Hz，20000 = 50Hz，40000 = 25Hz */
#ifndef BNO08X_REPORT_INTERVAL_US
#define BNO08X_REPORT_INTERVAL_US   20000UL
#endif

/* 是否使用 INT 引脚：0 = 不接（轮询），1 = 接 INT 后仅在中断信号有效时读取 */
#ifndef BNO08X_INT_ENABLE
#define BNO08X_INT_ENABLE      0
#endif

#ifndef BNO08X_INT_PORT
#define BNO08X_INT_PORT        GPIOB
#endif
#ifndef BNO08X_INT_PIN
#define BNO08X_INT_PIN         GPIO_PIN_12
#endif

/* 是否使用 RST 复位引脚：0 = 不接，1 = 初始化时硬件复位一次 BNO08X */
#ifndef BNO08X_RST_ENABLE
#define BNO08X_RST_ENABLE      0
#endif

#ifndef BNO08X_RST_PORT
#define BNO08X_RST_PORT        GPIOB
#endif
#ifndef BNO08X_RST_PIN
#define BNO08X_RST_PIN         GPIO_PIN_13
#endif

/* ---------------- 航向角标定 ---------------- */
/* 固定零偏（度）：例如传感器安装方向与车头相差 -30°，这里填 -30.0f */
#ifndef BNO08X_YAW_OFFSET_DEG
#define BNO08X_YAW_OFFSET_DEG  0.0f
#endif

/* 航向方向取反：0 = 正常，1 = yaw 取反（个别安装/坐标定义不同时使用） */
#ifndef BNO08X_YAW_INVERT
#define BNO08X_YAW_INVERT      0
#endif

/* ---------------- UART-RVC 模式参数 ---------------- */
/* RVC 模式波特率，BNO08X 数据手册规定为 115200，不要改 */
#ifndef BNO08X_RVC_BAUD
#define BNO08X_RVC_BAUD        115200UL
#endif

/* ======================================================================== */

/* 传感器数据（由 BNO08X_Service 实时刷新） */
typedef struct
{
    float    yaw;            /* 航向角 0~360°，0° = 磁北，顺时针增大   */
    float    pitch;          /* 俯仰角 -90~+90°                        */
    float    roll;           /* 横滚角 -180~+180°                      */
    uint8_t  accuracy;       /* 可信度：0=不可靠 1=低 2=中 3=高         */
    uint8_t  valid;          /* 1 = 已经收到过有效姿态数据              */
    uint8_t  online;         /* 1 = 最近 1 秒内收到过数据               */
    uint8_t  mode;           /* 1 = I2C(SH-2)，2 = UART-RVC             */
    uint8_t  i2c_addr;       /* I2C 模式：探测到的 7 位地址(0=未找到)  */
    uint16_t rate_hz;        /* 实测更新率（Hz）                        */
    uint32_t report_count;   /* 有效姿态帧累计数                        */
    uint32_t error_count;    /* 通信错误计数（校验错/读写错）           */
    uint32_t last_update_ms; /* 最后一次有效数据的时间戳（HAL_GetTick） */
} BNO08X_Data_t;

/**
  * @brief  初始化 BNO08X（在任务上下文中调用一次）
  * @note   I2C 模式下会延时等待模块启动，然后周期性发送使能命令；
  *         UART-RVC 模式下会配置 USART3 + DMA 循环接收。
  */
void BNO08X_Init(void);

/**
  * @brief  服务函数：读取/解析传感器数据
  * @note   必须在任务循环中高频调用（建议间隔 1~5ms），内部无阻塞等待。
  */
void BNO08X_Service(void);

/**
  * @brief  获取当前姿态数据指针（只读）
  */
const BNO08X_Data_t *BNO08X_GetData(void);

/**
  * @brief  设置航向角零偏（度），写入后立即生效
  */
void BNO08X_SetYawOffset(float offset_deg);

/**
  * @brief  把当前朝向标定为指定角度（例如把当前朝向当作 0°）
  * @param  target_deg 目标航向角（0~360）
  */
void BNO08X_SetHeadingReference(float target_deg);

/**
  * @brief  把航向角换算为 8 方位英文缩写，如 "N"/"NE"/"E"...
  */
const char *BNO08X_GetCompassDir(float heading_deg);

#ifdef __cplusplus
}
#endif

#endif /* __BNO08X_H__ */

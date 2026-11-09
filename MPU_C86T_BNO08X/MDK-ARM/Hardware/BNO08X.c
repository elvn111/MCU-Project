/**
  ******************************************************************************
  * @file    BNO08X.c
  * @brief   BNO08X (BNO080 / BNO085 / BNO086) 九轴姿态传感器驱动实现
  *
  *          两种工作模式（BNO08X_MODE 选择）：
  *
  *          [1] I2C 模式（默认）
  *              STM32F103 硬件 I2C2：PB10 = SCL，PB11 = SDA。
  *              BNO08X 使用 SHTP(Sensor Hub Transport Protocol) over I2C 传输：
  *                - 每次读：先读 4 字节 SHTP 包头，再按包头里的长度读载荷；
  *                - 每次写：包头 + 载荷一次写入；
  *                - 包头格式：byte0/byte1 = 长度(bit15 为续包标志)，
  *                            byte2 = 通道号，byte3 = 通道内序号。
  *              SH-2 协议流程：
  *                1) 上电等模块启动（约 100~300ms）；
  *                2) 在控制通道(2)发送“旋转矢量(0x05)”的 Set Feature 命令(0xFD)；
  *                3) 从输入通道(3)读取 0x05 报告，解析单位四元数(i,j,k,real)；
  *                4) 四元数 -> 欧拉角 -> 航向/俯仰/横滚。
  *              如果 1 秒收不到数据会自动重发使能命令（模块异常后可自恢复）。
  *
  *          [2] UART-RVC 模式
  *              USART3：PB10 = TX（可悬空），PB11 = RX，115200 8N1。
  *              模块 PS0 上电接 3V3 进入 RVC 模式后，以 100Hz 输出 19 字节帧：
  *                [0]=0xAA [1]=0xAA [2]=序号
  *                [3..4]=yaw   [5..6]=pitch [7..8]=roll     (int16 小端, 0.01°)
  *                [9..14]=加速度 x/y/z                       (int16 小端, 0.001g)
  *                [15..17]=保留  [18]=校验和(byte2~17 累加)
  *              USART3 的 RX 用 DMA1 通道3 循环接收，不占用 CPU 中断。
  *
  * @note    本文件不调用 printf，调试输出由应用层（freertos.c）完成。
  ******************************************************************************
  */

#include "BNO08X.h"

#include <string.h>
#include <math.h>

/* ============================== 公共部分 ============================== */

#define BNO08X_RAD2DEG      57.29577951f
#define BNO08X_QUAT_Q14     16384.0f     /* 旋转矢量四元数定点比例 2^14 */

static BNO08X_Data_t s_data;             /* 对外发布的姿态数据          */
static float    s_yaw_offset = BNO08X_YAW_OFFSET_DEG;
static uint32_t s_rate_t0 = 0u;          /* 更新率统计起始时间          */
static uint16_t s_rate_cnt = 0u;         /* 1 秒内的帧数                */

/* 角度归一化到 [0, 360) */
static float bn_norm360(float angle)
{
    while (angle >= 360.0f)
    {
        angle -= 360.0f;
    }
    while (angle < 0.0f)
    {
        angle += 360.0f;
    }
    return angle;
}

/* 四元数(单位, i/j/k/real) -> 欧拉角(度, ZYX 顺序) */
#if (BNO08X_MODE == BNO08X_MODE_I2C)
static void bn_quat_to_euler(float qi, float qj, float qk, float qr,
                             float *yaw, float *pitch, float *roll)
{
    float sin_pitch;

    /* pitch = asin(2*(real*j - k*i)) */
    sin_pitch = 2.0f * (qr * qj - qk * qi);
    if (sin_pitch > 1.0f)
    {
        sin_pitch = 1.0f;
    }
    else if (sin_pitch < -1.0f)
    {
        sin_pitch = -1.0f;
    }
    *pitch = asinf(sin_pitch) * BNO08X_RAD2DEG;

    /* roll = atan2(2*(real*i + j*k), 1 - 2*(i^2 + j^2)) */
    *roll = atan2f(2.0f * (qr * qi + qj * qk),
                   1.0f - 2.0f * (qi * qi + qj * qj)) * BNO08X_RAD2DEG;

    /* yaw = atan2(2*(real*k + i*j), 1 - 2*(j^2 + k^2)) */
    *yaw = atan2f(2.0f * (qr * qk + qi * qj),
                  1.0f - 2.0f * (qj * qj + qk * qk)) * BNO08X_RAD2DEG;
}
#endif

/* 把姿态写入对外数据结构（两种模式共用） */
static void bn_commit_attitude(float yaw, float pitch, float roll, uint8_t accuracy)
{
    uint32_t now = HAL_GetTick();
    float    yaw_applied;

#if (BNO08X_YAW_INVERT != 0)
    yaw_applied = -yaw;
#else
    yaw_applied = yaw;
#endif

    s_data.yaw   = bn_norm360(yaw_applied + s_yaw_offset);
    s_data.pitch = pitch;
    s_data.roll  = roll;
    s_data.accuracy = accuracy;
    s_data.valid    = 1u;
    s_data.online   = 1u;
    s_data.last_update_ms = now;
    s_data.report_count++;

    /* 每秒统计一次更新率 */
    s_rate_cnt++;
    if ((uint32_t)(now - s_rate_t0) >= 1000u)
    {
        s_data.rate_hz = s_rate_cnt;
        s_rate_cnt = 0u;
        s_rate_t0  = now;
    }
}

/* ============================== I2C 模式 ============================== */

#if (BNO08X_MODE == BNO08X_MODE_I2C)

#include "i2c.h"                         /* hi2c2 / MX_I2C2_Init */

#ifndef BNO08X_SHTP_CH_CONTROL
#define BNO08X_SHTP_CH_CONTROL   2u      /* SH-2 控制通道（发命令，标准固件固定为 2）*/
#endif
#ifndef BNO08X_SHTP_CH_INPUT
#define BNO08X_SHTP_CH_INPUT     3u      /* SH-2 输入通道（收报告，标准固件固定为 3）*/
#endif
#define BNO08X_SHTP_HDR_LEN      4u
#define BNO08X_RX_CAP            96u     /* 单帧最大缓存（字节）      */
#define BNO08X_I2C_TIMEOUT_MS    20u
#define BNO08X_REPORT_RV         0x05u   /* 旋转矢量报告 ID           */
#define BNO08X_CMD_SET_FEATURE   0xFD    /* Set Feature 命令          */
#define BNO08X_CMD_PAYLOAD_LEN   17u

static uint8_t  s_seq[8];                        /* 各通道发送序号     */
static uint8_t  s_i2c_addr = 0u;                 /* 探测到的 7 位地址   */
static uint32_t s_next_probe_ms = 0u;            /* 下次地址探测时刻   */
static uint8_t  s_tx_frame[BNO08X_SHTP_HDR_LEN + BNO08X_CMD_PAYLOAD_LEN];
static uint32_t s_start_ms = 0u;                 /* 驱动初始化时刻     */
static uint32_t s_next_set_feature_ms = 0u;      /* 下次重发使能命令   */

/* 探测某个 7 位地址上是否有器件应答（只发地址，不发数据） */
static uint8_t bn_i2c_probe(uint8_t addr7)
{
    return (HAL_I2C_IsDeviceReady(&hi2c2, (uint16_t)((uint16_t)addr7 << 1),
                                  2u, 10u) == HAL_OK) ? 1u : 0u;
}
/* 丢弃指定字节数（跳过被拆成多段的 SHTP 大包，保持数据流对齐） */
static uint8_t bn_i2c_discard(uint16_t count)
{
    uint8_t  tmp[16];
    uint16_t chunk;

    while (count > 0u)
    {
        chunk = (count > (uint16_t)sizeof(tmp)) ? (uint16_t)sizeof(tmp) : count;
        if (HAL_I2C_Master_Receive(&hi2c2, (uint16_t)((uint16_t)s_i2c_addr << 1),
                                   tmp, chunk, BNO08X_I2C_TIMEOUT_MS) != HAL_OK)
        {
            return 1u;
        }
        count = (uint16_t)(count - chunk);
    }
    return 0u;
}

/**
  * @brief  轮询读取一个完整的 SHTP 传输单元
  * @param  buf 接收缓存（含 4 字节包头）
  * @param  cap 缓存容量
  * @retval 帧长度（含包头）；0 表示当前没有数据或读取失败
  */
static uint16_t bn_i2c_read_frame(uint8_t *buf, uint16_t cap)
{
    uint8_t  hdr[BNO08X_SHTP_HDR_LEN];
    uint16_t len;
    uint8_t  cont;

    for (;;)
    {
        if (HAL_I2C_Master_Receive(&hi2c2, (uint16_t)((uint16_t)s_i2c_addr << 1),
                                   hdr, BNO08X_SHTP_HDR_LEN,
                                   BNO08X_I2C_TIMEOUT_MS) != HAL_OK)
        {
            return 0u;                   /* 总线错误或模块未响应 */
        }

        len  = (uint16_t)(((uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8)) & 0x7FFFu);
        cont = (uint8_t)((hdr[1] & 0x80u) ? 1u : 0u);

        if (len < BNO08X_SHTP_HDR_LEN)
        {
            return 0u;                   /* 模块暂无数据 */
        }

        if ((cont == 0u) && (len <= cap))
        {
            /* 单段完整包：读载荷 */
            if (len > BNO08X_SHTP_HDR_LEN)
            {
                if (HAL_I2C_Master_Receive(&hi2c2, (uint16_t)((uint16_t)s_i2c_addr << 1),
                                           &buf[BNO08X_SHTP_HDR_LEN],
                                           (uint16_t)(len - BNO08X_SHTP_HDR_LEN),
                                           BNO08X_I2C_TIMEOUT_MS) != HAL_OK)
                {
                    s_data.error_count++;
                    return 0u;
                }
            }
            memcpy(buf, hdr, BNO08X_SHTP_HDR_LEN);
            return len;
        }

        /* 多段包（例如开机广播）或超长包：丢弃本段内容 */
        if (bn_i2c_discard((uint16_t)(len - BNO08X_SHTP_HDR_LEN)) != 0u)
        {
            s_data.error_count++;
            return 0u;
        }
        if (cont == 0u)
        {
            return 0u;                   /* 整包已丢弃完毕 */
        }
    }
}

/* 发送一个 SHTP 包（单段） */
static void bn_i2c_send(uint8_t channel, const uint8_t *payload, uint16_t len)
{
    uint16_t total = (uint16_t)(len + BNO08X_SHTP_HDR_LEN);

    if (total > (uint16_t)sizeof(s_tx_frame))
    {
        return;
    }
    s_tx_frame[0] = (uint8_t)(total & 0xFFu);
    s_tx_frame[1] = (uint8_t)((total >> 8) & 0xFFu);
    s_tx_frame[2] = channel;
    s_tx_frame[3] = (uint8_t)(s_seq[channel & 0x07u]++);
    memcpy(&s_tx_frame[BNO08X_SHTP_HDR_LEN], payload, len);

    (void)HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)((uint16_t)s_i2c_addr << 1),
                                  s_tx_frame, total, BNO08X_I2C_TIMEOUT_MS);
}

/**
  * @brief  发送 SH-2 Set Feature 命令，使能指定报告周期上报
  * @param  report_id   报告 ID（旋转矢量 = 0x05）
  * @param  interval_us 上报周期（微秒）
  */
static void bn_i2c_set_feature(uint8_t report_id, uint32_t interval_us)
{
    uint8_t payload[BNO08X_CMD_PAYLOAD_LEN];

    memset(payload, 0, sizeof(payload));
    payload[0] = (uint8_t)BNO08X_CMD_SET_FEATURE;   /* 命令字          */
    payload[1] = report_id;                         /* 被使能的报告 ID */
    payload[2] = 0x00u;                             /* feature flags   */
    /* payload[3..4]：变化灵敏度，0 = 按固定周期上报              */
    payload[5] = (uint8_t)(interval_us & 0xFFu);    /* 上报周期 LSB    */
    payload[6] = (uint8_t)((interval_us >> 8) & 0xFFu);
    payload[7] = (uint8_t)((interval_us >> 16) & 0xFFu);
    payload[8] = (uint8_t)((interval_us >> 24) & 0xFFu);
    /* payload[9..12] ：批量间隔，0 = 不上报批量数据             */
    /* payload[13..16]：传感器专用配置，旋转矢量填 0 即可       */

    bn_i2c_send(BNO08X_SHTP_CH_CONTROL, payload, sizeof(payload));
}

/* 解析输入通道的 SH-2 报告 */
static void bn_i2c_handle_frame(const uint8_t *pkt, uint16_t len)
{
    uint8_t        channel;
    const uint8_t *report;
    int16_t        qi;
    int16_t        qj;
    int16_t        qk;
    int16_t        qr;
    float          yaw;
    float          pitch;
    float          roll;

    if (len < (BNO08X_SHTP_HDR_LEN + 4u))
    {
        return;
    }
    channel = (uint8_t)(pkt[2] & 0x0Fu);
    if (channel != BNO08X_SHTP_CH_INPUT)
    {
        return;                          /* 控制/广播/唤醒通道不处理 */
    }

    report = &pkt[BNO08X_SHTP_HDR_LEN];  /* SH-2 报告：report[0]=ID */
    if ((report[0] != BNO08X_REPORT_RV) || (len < (BNO08X_SHTP_HDR_LEN + 14u)))
    {
        return;
    }

    /* 旋转矢量报告：report[1]=序号 report[2]=状态 report[3]=时延
       report[4..11]=四元数 i/j/k/real（int16 小端，Q14），
       report[12..13]=精度（弧度，Q12） */
    qi = (int16_t)((uint16_t)report[4] | ((uint16_t)report[5] << 8));
    qj = (int16_t)((uint16_t)report[6] | ((uint16_t)report[7] << 8));
    qk = (int16_t)((uint16_t)report[8] | ((uint16_t)report[9] << 8));
    qr = (int16_t)((uint16_t)report[10] | ((uint16_t)report[11] << 8));

    bn_quat_to_euler((float)qi / BNO08X_QUAT_Q14,
                     (float)qj / BNO08X_QUAT_Q14,
                     (float)qk / BNO08X_QUAT_Q14,
                     (float)qr / BNO08X_QUAT_Q14,
                     &yaw, &pitch, &roll);

    /* 状态字节低 2 位即可信度：0=不可靠 1=低 2=中 3=高 */
    bn_commit_attitude(yaw, pitch, roll, (uint8_t)(report[2] & 0x03u));
}

/* 初始化可选引脚（INT / RST） */
static void bn_i2c_gpio_init(void)
{
#if (BNO08X_INT_ENABLE != 0) || (BNO08X_RST_ENABLE != 0)
    GPIO_InitTypeDef gpio = {0};
#endif

#if (BNO08X_INT_ENABLE != 0)
    gpio.Pin  = BNO08X_INT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BNO08X_INT_PORT, &gpio);
#endif

#if (BNO08X_RST_ENABLE != 0)
    gpio.Pin  = BNO08X_RST_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BNO08X_RST_PORT, &gpio);

    /* BNO08X 复位时序：拉低 10ms -> 拉高 -> 等待启动完成 */
    HAL_GPIO_WritePin(BNO08X_RST_PORT, BNO08X_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(BNO08X_RST_PORT, BNO08X_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(300);
#endif
}

void BNO08X_Init(void)
{
    memset(&s_data, 0, sizeof(s_data));
    memset(s_seq, 0, sizeof(s_seq));
    s_data.mode = 1u;                    /* I2C(SH-2) 模式 */
    s_yaw_offset = BNO08X_YAW_OFFSET_DEG;

    bn_i2c_gpio_init();

    s_i2c_addr = 0u;
    s_next_probe_ms = 0u;
    s_start_ms = HAL_GetTick();
    s_next_set_feature_ms = s_start_ms + 300u;   /* 等模块启动后再发命令 */
    s_data.i2c_addr = 0u;
}

void BNO08X_Service(void)
{
    uint8_t  pkt[BNO08X_RX_CAP];
    uint16_t n;
    uint8_t  i;
    uint32_t now;

    now = HAL_GetTick();
    if ((uint32_t)(now - s_start_ms) < 100u)
    {
        return;                          /* 上电后稍等再访问 I2C */
    }

    /* 上电自动探测 I2C 地址：0x4A(AD0 接地) 或 0x4B(AD0 接高)，每秒重试 */
    if (s_i2c_addr == 0u)
    {
        if ((int32_t)(now - s_next_probe_ms) >= 0)
        {
            uint8_t alt_addr;

            s_next_probe_ms = now + 200u;
            alt_addr = (BNO08X_I2C_ADDR == 0x4Au) ? 0x4Bu : 0x4Au;
            if (bn_i2c_probe(BNO08X_I2C_ADDR) != 0u)
            {
                s_i2c_addr = BNO08X_I2C_ADDR;
            }
            else if (bn_i2c_probe(alt_addr) != 0u)
            {
                s_i2c_addr = alt_addr;
            }
            s_data.i2c_addr = s_i2c_addr;
            if (s_i2c_addr != 0u)
            {
                s_next_set_feature_ms = now;    /* 找到模块后立即发使能命令 */
            }
        }
        if (s_i2c_addr == 0u)
        {
            s_data.online = 0u;
            return;                      /* 模块还没找到 */
        }
    }

    /* 一次最多读 4 帧，避免长时间占用任务 */
    for (i = 0u; i < 4u; i++)
    {
#if (BNO08X_INT_ENABLE != 0)
        /* INT 为低表示模块有数据待读（H_INTN 低有效） */
        if (HAL_GPIO_ReadPin(BNO08X_INT_PORT, BNO08X_INT_PIN) != GPIO_PIN_RESET)
        {
            break;
        }
#endif
        n = bn_i2c_read_frame(pkt, (uint16_t)sizeof(pkt));
        if (n == 0u)
        {
            break;
        }
        bn_i2c_handle_frame(pkt, n);
    }

    /* 还没收到数据或超过 1 秒没有数据：周期性重发使能命令，模块异常后可自动恢复 */
    if ((s_data.valid == 0u) || ((uint32_t)(now - s_data.last_update_ms) > 1000u))
    {
        if ((int32_t)(now - s_next_set_feature_ms) >= 0)
        {
            bn_i2c_set_feature(BNO08X_REPORT_RV, BNO08X_REPORT_INTERVAL_US);
            s_next_set_feature_ms = now + 500u;
        }
    }

    s_data.online = ((uint32_t)(now - s_data.last_update_ms) < 1000u) ? 1u : 0u;
}

/* ============================ UART-RVC 模式 ============================ */

#elif (BNO08X_MODE == BNO08X_MODE_UART_RVC)

#include "i2c.h"                         /* 用于关闭与 USART3 复用的 I2C2 */

#define BNO08X_RVC_BUF_SIZE      1024u   /* DMA 循环接收缓冲（约 89ms 数据）*/
#define BNO08X_RVC_FRAME_SIZE    19u     /* RVC 固定帧长                */
#define BNO08X_RVC_YAW_SCALE     0.01f   /* yaw/pitch/roll 单位 0.01°   */

static UART_HandleTypeDef s_rvc_uart;
static DMA_HandleTypeDef  s_rvc_dma;
static uint8_t  s_rvc_buf[BNO08X_RVC_BUF_SIZE];
static uint16_t s_rvc_rd = 0u;           /* 软件读指针                  */
static uint8_t  s_rvc_frame[BNO08X_RVC_FRAME_SIZE];
static uint8_t  s_rvc_cnt = 0u;          /* 当前帧已收字节数            */
static uint8_t  s_rvc_hw_ok = 0u;

/* 配置 USART3(PB10/PB11) + DMA1 通道3 循环接收 */
static void bn_rvc_hw_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 原工程 PB10/PB11 是 I2C2（接 MPU6050），RVC 模式下改作 USART3：
       PB10 = USART3_TX，PB11 = USART3_RX */
    HAL_I2C_DeInit(&hi2c2);

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    gpio.Pin   = GPIO_PIN_10;
    gpio.Mode  = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin  = GPIO_PIN_11;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);

    s_rvc_uart.Instance          = USART3;
    s_rvc_uart.Init.BaudRate     = BNO08X_RVC_BAUD;
    s_rvc_uart.Init.WordLength   = UART_WORDLENGTH_8B;
    s_rvc_uart.Init.StopBits     = UART_STOPBITS_1;
    s_rvc_uart.Init.Parity       = UART_PARITY_NONE;
    s_rvc_uart.Init.Mode         = UART_MODE_TX_RX;
    s_rvc_uart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    s_rvc_uart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&s_rvc_uart) != HAL_OK)
    {
        s_data.error_count++;
        return;
    }

    s_rvc_dma.Instance                 = DMA1_Channel3;   /* USART3_RX 通道 */
    s_rvc_dma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    s_rvc_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    s_rvc_dma.Init.MemInc              = DMA_MINC_ENABLE;
    s_rvc_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    s_rvc_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    s_rvc_dma.Init.Mode                = DMA_CIRCULAR;
    s_rvc_dma.Init.Priority            = DMA_PRIORITY_HIGH;
    if (HAL_DMA_Init(&s_rvc_dma) != HAL_OK)
    {
        s_data.error_count++;
        return;
    }
    __HAL_LINKDMA(&s_rvc_uart, hdmarx, s_rvc_dma);

    if (HAL_DMA_Start(&s_rvc_dma, (uint32_t)&USART3->DR,
                      (uint32_t)s_rvc_buf, BNO08X_RVC_BUF_SIZE) != HAL_OK)
    {
        s_data.error_count++;
        return;
    }
    SET_BIT(USART3->CR3, USART_CR3_DMAR);    /* UART 接收请求交给 DMA */
    __HAL_UART_ENABLE(&s_rvc_uart);

    s_rvc_hw_ok = 1u;
}

/* 解析一帧完整的 RVC 数据 */
static void bn_rvc_parse_frame(const uint8_t *f)
{
    int16_t yaw_raw;
    int16_t pitch_raw;
    int16_t roll_raw;

    /* f[0..1] = 0xAA 0xAA  f[2] = 序号
       f[3..14] = yaw/pitch/roll/加速度 (int16 小端)
       f[15..17] = 保留  f[18] = 校验和 */
    yaw_raw   = (int16_t)((uint16_t)f[3] | ((uint16_t)f[4] << 8));
    pitch_raw = (int16_t)((uint16_t)f[5] | ((uint16_t)f[6] << 8));
    roll_raw  = (int16_t)((uint16_t)f[7] | ((uint16_t)f[8] << 8));

    /* RVC 模式没有可信度字段，按“高”处理 */
    bn_commit_attitude((float)yaw_raw * BNO08X_RVC_YAW_SCALE,
                       (float)pitch_raw * BNO08X_RVC_YAW_SCALE,
                       (float)roll_raw * BNO08X_RVC_YAW_SCALE,
                       3u);
}

/* 逐字节状态机：找 0xAA 0xAA 帧头 -> 收满 19 字节 -> 校验 */
static void bn_rvc_feed(uint8_t byte)
{
    uint8_t sum;
    uint8_t i;

    if (s_rvc_cnt < 2u)
    {
        if (byte == 0xAAu)
        {
            s_rvc_frame[s_rvc_cnt++] = byte;
        }
        else
        {
            s_rvc_cnt = 0u;
        }
        return;
    }

    s_rvc_frame[s_rvc_cnt++] = byte;
    if (s_rvc_cnt < BNO08X_RVC_FRAME_SIZE)
    {
        return;
    }
    s_rvc_cnt = 0u;

    /* 校验和 = 序号+数据+保留字节 的累加（不含两个 0xAA） */
    sum = 0u;
    for (i = 2u; i < (BNO08X_RVC_FRAME_SIZE - 1u); i++)
    {
        sum = (uint8_t)(sum + s_rvc_frame[i]);
    }
    if (sum == s_rvc_frame[BNO08X_RVC_FRAME_SIZE - 1u])
    {
        bn_rvc_parse_frame(s_rvc_frame);
    }
    else
    {
        s_data.error_count++;
    }
}

void BNO08X_Init(void)
{
    memset(&s_data, 0, sizeof(s_data));
    s_data.mode = 2u;                    /* UART-RVC 模式 */
    s_yaw_offset = BNO08X_YAW_OFFSET_DEG;
    s_rvc_rd  = 0u;
    s_rvc_cnt = 0u;
    s_rvc_hw_ok = 0u;

    bn_rvc_hw_init();
}

void BNO08X_Service(void)
{
    uint16_t pos;
    uint32_t now;

    if (s_rvc_hw_ok == 0u)
    {
        return;
    }

    /* DMA 循环缓冲：取当前写指针，消费读指针到写指针之间的数据 */
    pos = (uint16_t)(BNO08X_RVC_BUF_SIZE - __HAL_DMA_GET_COUNTER(&s_rvc_dma));
    while (s_rvc_rd != pos)
    {
        bn_rvc_feed(s_rvc_buf[s_rvc_rd]);
        s_rvc_rd++;
        if (s_rvc_rd >= BNO08X_RVC_BUF_SIZE)
        {
            s_rvc_rd = 0u;
        }
    }

    now = HAL_GetTick();
    s_data.online = ((uint32_t)(now - s_data.last_update_ms) < 1000u) ? 1u : 0u;
}

#else
#error "BNO08X_MODE 配置错误：只能是 BNO08X_MODE_I2C 或 BNO08X_MODE_UART_RVC"
#endif /* BNO08X_MODE */

/* ============================ 公共接口实现 ============================ */

const BNO08X_Data_t *BNO08X_GetData(void)
{
    return &s_data;
}

void BNO08X_SetYawOffset(float offset_deg)
{
    s_yaw_offset = offset_deg;
}

void BNO08X_SetHeadingReference(float target_deg)
{
    float delta;
    float current;

    if (s_data.valid == 0u)
    {
        return;                          /* 还没有数据，先等一帧 */
    }

    /* 以“当前显示航向”为基准，把零偏调整到目标角度 */
    current = s_data.yaw;
    delta = target_deg - current;
    s_yaw_offset += delta;

    /* 防止零偏无限增大 */
    while (s_yaw_offset >= 3600.0f)
    {
        s_yaw_offset -= 3600.0f;
    }
    while (s_yaw_offset <= -3600.0f)
    {
        s_yaw_offset += 3600.0f;
    }
}

const char *BNO08X_GetCompassDir(float heading_deg)
{
    static const char *const dirs[8] = {"N", "NE", "E", "SE",
                                        "S", "SW", "W", "NW"};
    uint16_t index;

    heading_deg = bn_norm360(heading_deg);
    /* +22.5° 让每个方位覆盖 45° 范围，四舍五入到最近方位 */
    index = (uint16_t)((heading_deg + 22.5f) / 45.0f);
    index = (uint16_t)(index % 8u);
    return dirs[index];
}

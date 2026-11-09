# MPU_C86T_BNO08X —— 原 MPU6050 工程改为适配 BNO08X（九轴 IMU）

> 本工程由 `Desktop\stm32cubemx project\MPU_C86T_ture\MPU_C86T_ture` **另存**而来，
> 原工程未做任何改动。另存后的位置：
> `Desktop\deepseek_WorkSpace\MPU_C86T_BNO08X`
>
> 原工程功能：MPU6050 加速度计 → OLED 画“地平线”。
> 新工程功能：**BNO08X → 航向角（yaw）/ 俯仰角（pitch）/ 横滚角（roll）→ OLED 显示 + 串口打印**。

---

## 1. 硬件平台与工程信息

| 项目 | 内容 |
|---|---|
| MCU | STM32F103C8T6（64KB Flash / 20KB RAM） |
| 系统时钟 | HSI 8MHz（与 CubeMX 配置一致，未使用 PLL） |
| 工程 | `MDK-ARM\MPU_C86T_BNO08X.uvprojx`（Keil MDK，ARMCC V5.06） |
| 系统框架 | CubeMX + FreeRTOS（CMSIS-RTOS V2） |
| 已有外设 | I2C2(PB10/PB11)、USART2(PA2/PA3, 9600)、ADC1+DMA1_CH1、OLED 软 I2C(PB6/PB7) + OLED 供电脚(PB4/PB5) |
| 新增 | `MDK-ARM\Hardware\BNO08X.c/.h`：BNO08X 驱动（I2C SHTP/SH-2 与 UART-RVC 双模式） |

编译输出（已验证）：`firmware\MPU_C86T_BNO08X_I2C.hex`、`firmware\MPU_C86T_BNO08X_RVC.hex`。

---

## 2. 快速开始（默认 I2C 接法）

1. 用 Keil 打开 `MDK-ARM\MPU_C86T_BNO08X.uvprojx`，Rebuild，下载。
2. BNO08X 模块按下面接线（和原来 MPU6050 的 4 线位置一样）：

| BNO08X 模块 | STM32F103 板 |
|---|---|
| VCC / 3V3 | 3.3V（**不要接 5V**） |
| GND | GND |
| SCL | PB10（原 MPU6050 SCL） |
| SDA | PB11（原 MPU6050 SDA） |
| INT（可不接） | 默认不用（程序轮询读取）；要接可改成 PB12，见第 5 节 |

3. 上电后 OLED 应出现 `BNO08X I2C` → 约 1 秒内变成 `HDG xxx.x xx`。
   同时 USART2（9600 8N1）每秒打印一次 `[BNO08X] H=... P=... R=...`。

> 模块地址：AD0/SA0 引脚接地 = 0x4A（多数模块默认），接高 = 0x4B。
> **程序会自动探测 0x4A / 0x4B 两个地址**，一般不用改。

---

## 3. 两种接法，二选一

BNO08X 支持多种接口，本工程实现了其中两种，通过 `MDK-ARM\Hardware\BNO08X.h` 顶部的
`BNO08X_MODE` 宏选择（默认 `BNO08X_MODE_I2C`）。

### 3.1 I2C 模式（默认，推荐）

- 使用 STM32 硬件 I2C2：PB10=SCL、PB11=SDA，400kHz。
- 采用 BNO08X 的 **SHTP + SH-2** 协议：
  1. 上电后自动探测地址（0x4A/0x4B）；
  2. 在控制通道发送 Set Feature(0xFD)，使能“旋转矢量(0x05)”上报（默认 20ms/50Hz）；
  3. 从输入通道读取 0x05 报告，解析四元数 (i,j,k,real, Q14)，换算欧拉角；
  4. 1 秒收不到数据会自动重发使能命令，模块异常后能自恢复。
- INT 引脚：默认**不接**也可以（驱动采用“读 SHTP 包头轮询”的方式）。
  如果模块 INT 已接到 MCU，把 `BNO08X_INT_ENABLE` 改为 1，并按实际改 `BNO08X_INT_PORT/PIN`，可明显减少 I2C 流量。
- RST 引脚：可选。若模块 RST 接到 MCU，把 `BNO08X_RST_ENABLE` 改为 1 并改端口/引脚，初始化时硬件复位一次。
- 上拉电阻：模块板一般自带；若接线较长或通信不稳，SCL/SDA 各加 4.7kΩ 上拉到 3.3V。

### 3.2 UART-RVC 模式（最简单，但需要模块进入 RVC 模式）

BNO08X 的 **UART-RVC** 模式会以 100Hz 主动输出 19 字节固定帧，无需发送任何指令：

```
字节: [0]=0xAA [1]=0xAA [2]=序号
      [3..4]=yaw   [5..6]=pitch [7..8]=roll   (int16 小端, 单位 0.01°)
      [9..14]=加速度 x/y/z                     (int16 小端, 单位 0.001g)
      [15..17]=保留  [18]=校验和（字节 2~17 累加）
```

接线与设置：

1. 模块 **上电时把 PS0 接 3V3**（Adafruit 的 BNO08x 板：把背面 `P0` 焊盘短接/飞线到 3V；其他模块找 `PS0` 或 `P0` 脚）。
2. 模块的数据输出脚（Adafruit 板丝印是 `SDA`，部分模块标 `TX`）→ **PB11（USART3_RX）**；GND 与板子共地；VCC 3.3V。
3. 打开 `BNO08X.h`，把
   `#define BNO08X_MODE            BNO08X_MODE_I2C`
   改成
   `#define BNO08X_MODE            BNO08X_MODE_UART_RVC`
4. 重新编译下载。程序会自动 `HAL_I2C_DeInit()` 释放 PB10/PB11，再把 PB11 配成 USART3_RX（115200 8N1，DMA1_CH3 循环接收）。

> 注意：I2C 模式与 RVC 模式互斥，模块不能同时工作在两种模式；
> 使用 RVC 时 PB10 被配成 USART3_TX（本工程不发数据，可悬空不接）。

---

## 4. 屏幕显示与串口输出

OLED（4 行，8×16 字体，5Hz 刷新）：

```
HDG 123.4 NW      ← 第1行：航向角（0~360°）+ 8 方位
PIT +12.3         ← 第2行：俯仰角（-90~+90°）
ROL -45.6         ← 第3行：横滚角（-180~+180°）
ACC:3 50Hz        ← 第4行：可信度 0~3 + 实测更新率
```

- 没有数据时第 1 行显示 `BNO08X I2C` / `BNO08X RVC`，第 2~4 行显示排查提示。
- 方位缩写：N / NE / E / SE / S / SW / W / NW。

USART2（PA2=TX，**9600 8N1**）每秒打印：

```
[BNO08X] H=123.4 P=+12.3 R=-45.6
```

> 原工程里 `printf` 会同时画到 OLED 并每字符 `HAL_Delay(10)`，这里已改成只输出串口，
> 否则会严重拖慢传感器读取。

---

## 5. 参数配置（`MDK-ARM\Hardware\BNO08X.h`）

| 宏 | 默认值 | 说明 |
|---|---|---|
| `BNO08X_MODE` | `BNO08X_MODE_I2C` | 二选一：`BNO08X_MODE_I2C` / `BNO08X_MODE_UART_RVC` |
| `BNO08X_I2C_ADDR` | `0x4A` | 优先探测的 7 位地址；另一个地址会自动探测 |
| `BNO08X_REPORT_INTERVAL_US` | `20000` | 旋转矢量上报周期（微秒）：20000=50Hz，10000=100Hz |
| `BNO08X_INT_ENABLE` | `0` | 1=用 INT 引脚门控读取 |
| `BNO08X_INT_PORT/PIN` | `GPIOB` / `GPIO_PIN_12` | INT 实际引脚（仅 INT_ENABLE=1 时有效） |
| `BNO08X_RST_ENABLE` | `0` | 1=初始化时硬件复位 BNO08X |
| `BNO08X_RST_PORT/PIN` | `GPIOB` / `GPIO_PIN_13` | RST 实际引脚（仅 RST_ENABLE=1 时有效） |
| `BNO08X_YAW_OFFSET_DEG` | `0.0f` | 航向固定零偏（度），用于安装方向补偿 |
| `BNO08X_YAW_INVERT` | `0` | 1=航向取反（方向相反时用） |
| `BNO08X_RVC_BAUD` | `115200` | RVC 波特率，数据手册规定值，不要改 |

运行时还可用：

```c
BNO08X_SetHeadingReference(0.0f);   /* 把当前朝向标定为 0°（车头/机头方向） */
BNO08X_SetYawOffset(-30.0f);        /* 直接设置零偏 */
```

---

## 6. 航向角说明与校准（重要）

- 航向角来自 BNO08X 的**旋转矢量 (0x05)**，0° 对应磁北（不是地理北），顺时针增大。
- 第一次使用或换环境后，需要做**磁力计校准**：拿起模块在空中缓慢画“8”字或水平转几圈，
  直到 OLED 第 4 行的 `ACC` 从 0/1 升到 2/3。`ACC` 含义：0=不可靠 1=低 2=中 3=高。
- 安装方向与参考方向不一致时：
  - 固定补偿：改 `BNO08X_YAW_OFFSET_DEG`；
  - 运行时标定：调用 `BNO08X_SetHeadingReference(0.0f)`（把当前朝向当 0°）。
- 若转动方向和显示相反，把 `BNO08X_YAW_INVERT` 设为 1（或零偏里加负号处理）。
- 远离磁铁、电机、大电流走线，否则航向会漂。

> RVC 模式的 yaw/pitch/roll 也是芯片内部融合后的结果，校准方法相同。

---

## 7. 相比原工程的改动清单

| 文件 | 改动 |
|---|---|
| `MDK-ARM\Hardware\BNO08X.c/.h` | **新增**：BNO08X 驱动，I2C SHTP/SH-2 + UART-RVC 双模式，输出 yaw/pitch/roll |
| `MDK-ARM\Hardware\IIC_Hardware.c/.h` | 原 MPU6050 驱动，已从 Keil 工程移除（**文件保留**，仅供对照，不参与编译） |
| `Core\Src\main.c` | 删除 MPU6050 初始化；`fputc` 简化为只发串口（原实现每字符延时 10ms 并刷 OLED）；开机显示 `BNO08X IMU / INIT` |
| `Core\Src\freertos.c` | `myTask02` 改为：`BNO08X_Init()` → 循环 `BNO08X_Service()` + OLED 5Hz + 串口 1Hz；任务栈 128→384 words；新增显示/打印辅助函数 |
| `Core\Inc\FreeRTOSConfig.h` | `configTOTAL_HEAP_SIZE` 3072 → 6144（任务栈变大后需要） |
| `MDK-ARM\Hardware\OLED.c/.h` | 新增 `OLED_ShowFloat()`（定点小数显示）；`OLED_Horizon()` 保留未用 |
| `MDK-ARM\*.uvprojx/.uvoptx`、`*.ioc` | 工程名改为 `MPU_C86T_BNO08X`；HardWare 组用 `BNO08X.c/h` 替换 `IIC_Hardware.c/h`；`.ioc` 硬件配置未改（USART3/INT/RST 由驱动代码配置） |
| `_tools\build_armcc.ps1` | 新增：用 Keil 自带 ARMCC 命令行编译/链接本工程（见第 8 节） |
| `firmware\*.hex` | 两种模式编译出的 hex，可直接烧录验证 |

> 如果以后用 STM32CubeMX 重新生成代码：硬件配置（I2C2/USART2/OLED 引脚）不会变，
> 但需要保留 USER CODE 段内容，并把 `BNO08X.c/.h` 重新加回 Keil 的 HardWare 组。

---

## 8. 编译验证说明

本机当前会话无法启动 μVision(UV4) 的批处理编译（GUI 进程被系统直接终止），
因此使用 **Keil MDK 自带的 ARMCC V5.06 工具链命令行**做了完整编译 + 链接验证：

```powershell
# 默认 I2C 模式（结果见 MDK-ARM\Objects_armcc_I2C）
powershell -ExecutionPolicy Bypass -File .\_tools\build_armcc.ps1 -Mode I2C

# UART-RVC 模式（结果见 MDK-ARM\Objects_armcc_RVC）
powershell -ExecutionPolicy Bypass -File .\_tools\build_armcc.ps1 -Mode RVC
```

验证结果（两种模式均 0 error / 0 warning）：

| 模式 | Code+RO | RW+ZI(RAM) | 说明 |
|---|---|---|---|
| I2C | 约 23.6 KB | 约 10.8 KB | Flash 64KB / RAM 20KB，余量充足 |
| UART-RVC | 约 20.0 KB | 约 11.9 KB | 同上 |

> `MDK-ARM\Objects_armcc_*` 是命令行验证的中间产物，可删除。
> 用 Keil 图形界面打开工程后直接 Rebuild 即可，工程本身配置正常。

---

## 9. 常见问题排查

**OLED 一直显示 `BNO08X I2C / NO DATA`**
1. 模块供电是否为 3.3V、GND 是否共地；
2. SCL/SDA 是否接反；PB10=SCL、PB11=SDA；
3. SCL/SDA 是否需要上拉（模块自带则不用）；
4. 模块地址：程序会自动试 0x4A/0x4B，若模块 AD0 悬空，先把 AD0 接地或接 3V3；
5. 用逻辑分析仪/示波器看 I2C 是否有 ACK；
6. 若模块 INT 悬空且轮询不稳定，把 INT 接到 PB12 并设 `BNO08X_INT_ENABLE=1` 试试。

**OLED 一直显示 `BNO08X RVC / NO DATA`**
1. 模块是否在**上电时**被置为 RVC 模式（PS0=3V3）；运行中再改无效，需要重新上电；
2. 模块输出脚是否接到 PB11（USART3_RX），GND 是否共地；
3. 波特率是否 115200；
4. 可用 USB-TTL 直接接模块输出脚，串口助手 115200 查看是否有 `AA AA` 开头的二进制帧。

**有数据但航向变化/漂移**
- 做磁力计校准（画 8 字），检查远离磁干扰源；
- `ACC` 一直为 0 说明磁力计未校准好。

**航向方向相反或零点不对**
- `BNO08X_YAW_INVERT` 或 `BNO08X_YAW_OFFSET_DEG` / `BNO08X_SetHeadingReference()`。

**编译报错找不到 BNO08X.h**
- 确认文件在 `MDK-ARM\Hardware\` 下，且 Keil 的 Include Paths 含 `./Hardware`（工程已配好）。

---

## 10. 目录结构（主要文件）

```
MPU_C86T_BNO08X\
├─ Core\                        CubeMX 生成代码（main.c / freertos.c / i2c.c / usart.c ...）
├─ Drivers\                     HAL + CMSIS
├─ Middlewares\                 FreeRTOS
├─ MDK-ARM\
│  ├─ MPU_C86T_BNO08X.uvprojx   Keil 工程（打开这个）
│  ├─ Hardware\
│  │  ├─ BNO08X.c / BNO08X.h    ★ 新增：BNO08X 驱动（两种模式）
│  │  ├─ OLED.c / OLED.h / OLED_Font.h
│  │  └─ IIC_Hardware.c/.h      原 MPU6050 驱动（已不参与编译，保留备查）
│  ├─ Objects_armcc_I2C\        命令行验证产物（可删）
│  └─ Objects_armcc_RVC\        命令行验证产物（可删）
├─ _tools\build_armcc.ps1       命令行编译脚本
├─ firmware\                    可直接烧录的 hex
├─ MPU_C86T_BNO08X.ioc          CubeMX 工程文件（硬件配置未改）
└─ README_BNO08X适配说明.md      本文件
```

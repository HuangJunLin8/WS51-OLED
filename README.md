# OLED_IIC

WS51F6240 驱动 0.69 寸 OLED (SSD1306, 96×16 像素)，当前使用**软件 IIC（GPIO 模拟）**，带轻量字体渲染。

<img width="496" height="272" alt="VID_20260714_235943" src="https://github.com/user-attachments/assets/6fe60286-0d46-46be-8b82-2b7c4028f54c" />

## 当前状态

| 项目 | 状态 | 说明 |
|------|------|------|
| 软件 IIC | ✅ 正常 | P13(SCL) + P14(SDA)，GPIO 模拟，~40kHz |
| 基本显示 | ✅ 正常 | 矩形框、文字 `WS51F6240` 交替闪烁 |
| 字体渲染 | ✅ 正常 | Terminus_14 字体，移植自 u8g2 的 BDF 解码器 |
| 硬件 IIC | ❌ 未实现 | 曾尝试切到 P02/P16，但尚未调通 |



## 内存占用

**软件IIC**：

| 类型      | 大小               | 说明                             |
| :-------- | :----------------- | :------------------------------- |
| **data**  | 9.0 字节           | 内部RAM（0x00-0x7F）直接寻址数据 |
| **xdata** | 303 字节           | 外部扩展RAM（单片机内部XDATA区） |
| **code**  | 6228 字节 (6.08KB) | 程序代码占用的Flash空间          |



## 硬件连接

| OLED 引脚 | WS51F6240 引脚 | 说明 |
|-----------|---------------|------|
| SCL       | P13           | 软件 IIC 时钟（推挽输出） |
| SDA       | P14           | 软件 IIC 数据（开漏，外部 4.7kΩ 上拉） |
| VCC       | 3.3V          | |
| GND       | GND           | |
| RST       | 接复位电路  | ⚠️ 无该电路上电容易乱码 |

> **注意**: P02/P16 是 WS51F6240 的硬件 IIC 引脚，同时也是 I2C 调试下载接口。若配置硬件IIC，默认上电需要加入 64ms 延时用于下载握手，如果屏蔽了该延时，下次无法烧录。



## 已知问题

### OLED 上电乱码（已解决）

**现象**: 上电后 OLED 可能显示乱码（随机噪点），拔插 FPC 排线有时能恢复，有时依旧。

**分析**: 很可能是 RST 引脚直接焊到 3.3V，缺少正确的 RC 复位电路。SSD1306 数据手册要求 RST 引脚在上电后保持低电平至少 3μs 再拉高，直接接高电平会导致内部寄存器未正确初始化。

**已验证方案**: 将 RST 接 RC 延时电路（如 4.7kΩ + 4.7μF），上电后先拉低再拉高。

**OLED 电路**: 添加了复位电路，绘制 PCB 时要注意线序，小心画反。

<img width="1442" height="1058" alt="image" src="https://github.com/user-attachments/assets/590a23dd-400a-47c2-b881-b91814ede496" />

否则得像我这样搭棚+飞线大法

<img width="4096" height="2300" alt="2026-07-15_13-44-24_817" src="https://github.com/user-attachments/assets/b8ecf5fd-be63-4a70-bff2-d4e1c5a54b81" />

> 吐了 FPC 还画反了

## 项目结构

```
OLED_IIC/
├── Core/
│   ├── main.c                       # 主程序（系统时钟、定时器、测试循环）
│   └── BSP/OLED/
│       ├── oled_i2c.c / .h          # 软件 I2C 驱动（GPIO 模拟）
│       ├── oled_disp.c / .h         # 显示驱动（帧缓冲区、SSD1306 初始化、绘制 API）
├── Inc/
│   ├── WS51F6240.h                  # 芯片寄存器头文件
│   └── stdint.h                     # C51 兼容的 stdint
├── Libs/
│   └── Font/
│       ├── font.c / .h              # u8g2 BDF 字体解码器（C51 适配）
│       └── font_data.c              # 字体数据（Terminus_14）
├── Scripts/
│   ├── keil_add_file.py             # Keil 工程文件批量添加脚本
│   └── format_comments.py           # 注释格式化脚本
├── STARTUP.A51                      # 8051 启动文件
├── OLED_IIC.uvproj                  # Keil C51 工程
└── README.md
```

## 软件架构

```
main.c
 ├── init_timer0()          → 系统嘀嗒 (1ms, 基于 Timer0)
 ├── delay_ms()             → 毫秒延时
 ├── draw_rect()            → 矩形框绘制
 └── OLED_Init() / OLED_Clear() / OLED_Flush() / OLED_DrawString()

oled_disp.c (显示驱动)
 ├── g_oled_fb[192]         → XDATA 帧缓冲区 (2 pages × 96 columns)
 ├── OLED_Init()            → 软件 I2C 初始化 + SSD1306 命令序列 + 字体初始化
 ├── OLED_Clear()           → 清空帧缓冲区
 ├── OLED_Flush()           → 页寻址模式逐页刷屏
 ├── OLED_SetPixel()        → 设置/清除单个像素
 ├── OLED_DrawHLine()       → 水平线段（供字体解码器回调）
 └── OLED_DrawString()      → 绘制 ASCII 字符串

oled_i2c.c (软件 I2C 驱动)
 ├── IIC_Start/Stop         → 起始/停止信号
 ├── IIC_Send_Byte          → 发送单字节
 ├── IIC_Wait_Ack           → 等待从机 ACK（带超时保护）
 └── OLED_I2C_Send          → 公开 API: [设备地址] + [控制字节] + [数据]

font.c (字体解码器)
 ├── Font_Init/SetType      → 初始化/切换字体
 ├── Font_DrawStr           → 绘制 ASCII 字符串
 └── 字体数据格式           → u8g2 BDF 编译格式 (23 字节头部 + RLE 编码字形)
```

## 帧缓冲区布局

- **大小**: 192 字节，存放在 XDATA 区
- **索引**: `g_oled_fb[page * 96 + col]`
- **像素映射**: `(x, y)` → `idx = (y/8)*96 + x`, `bit = y % 8`（bit 0 = 页内顶部像素）

## I2C 通信帧格式

SSD1306 每次写入需发送控制字节区分命令和显示数据：

```
[设备地址 0x78] + [控制字节] + [数据...]
```

| 控制字节 | 含义 |
|----------|------|
| `0x00`   | 后续是命令 |
| `0x40`   | 后续是显示数据 (写入 GDDRAM) |

## 时钟配置

- 系统时钟: HRC 16MHz
- Timer0 重载值: 65536 - 1334 ≈ 1ms 中断
- I2C 时钟: ~40kHz（`IIC_CLK_ADJ = 1`，`delay_us()` 空循环）

---



## Scripts 工具说明

### keil_add_file.py

在 Keil v5 工程中**批量添加源文件**和**头文件搜索路径**，避免手动逐个添加。

#### 前提条件

1. 脚本必须放在 **Keil 工程同级目录**下
2. 工程文件必须是 XML 格式（`.uvproj` 或 `.uvprojx`），Keil v5 默认即 XML
3. 如需添加到指定分组，**先手动在 Keil 中创建好空的分组**（Group）

#### 使用方式

##### 1. 在 Keil 中创建分组

打开工程 → 右键 `Target 1` → `Add Group...` → 输入分组名（如 `Font`）

##### 2. 运行脚本

在终端中执行：

```bash
python keil_add_file.py
```

##### 3. 输入参数

脚本提示 `请输入参数:` 后，按格式输入三个参数（空格分隔）：

```
<模式> <文件夹相对路径> <分组名>
```

| 参数 | 说明 | 示例 |
|------|------|------|
| 模式 | `0`=全部，`1`=只加 .c，`2`=只加 .h 路径 | `0` |
| 路径 | 要添加的文件夹，**相对于工程目录** | `.\Libs\Font` |
| 分组名 | 目标分组（需在 Keil 中先建好） | `Font` |

##### 4. 示例

将 `Libs\Font` 目录下的所有 `.c` 文件加到 `Font` 分组，同时将所有子目录加入头文件搜索路径：

```
请输入参数: 0 .\Libs\Font Font
```

只加头文件路径（不加 .c 文件）：

```
请输入参数: 2 .\Libs\Font Font
```

#### 工作原理

1. 找到当前目录唯一的 `.uvproj` 或 `.uvprojx` 文件
2. 将其重命名为 `.xml`，用 XML 解析器修改
3. `.c` 文件 → 在目标 `<Group>` 下创建 `<File>` 节点（去重）
4. `.h` 路径 → 追加到编译器 `<IncludePath>`（去重）
5. 保存后重命名回原扩展名

#### 注意事项

- 路径参数用**反斜杠** `\` 而非 `/`（Windows 下 Keil 使用反斜杠）
- 如果提示"未发现头文件分组"，说明工程格式不被识别 —— 本脚本已适配 Keil C51（`<C51>`）和 Keil MDK（`<Cads>`）两种格式
- 脚本会**跳过已存在的文件和路径**，不会重复添加
- 运行后需要在 Keil 中刷新或重新打开工程才能看到变化

#### 故障排除

| 问题 | 原因 | 解决 |
|------|------|------|
| 未找到工程文件 | 脚本不在工程目录 | 把脚本复制到 `.uvproj` 同级目录 |
| 找到多个工程文件 | 目录下有多个 uvproj | 只保留一个 |
| 未发现 XX 分组 | 分组名拼写错误或未创建 | 先在 Keil 中手动创建该分组 |
| 卡住不动 | `Event().wait()` 导致 | 按 Ctrl+C 或关掉终端窗口即可 |

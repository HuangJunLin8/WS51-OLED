/**
 * oled_i2c.c — SSD1306 软件 I2C 驱动实现
 *
 * 参考 SDK "19-IIC-host-模拟"，GPIO 模拟 I2C 主机:
 *   P13 = SCL (推挽输出)
 *   P14 = SDA (开漏，输入/输出自动切换)
 *
 * 注意: SDA 需外部 4.7kΩ 上拉电阻
 */

#include "oled_i2c.h"

// -------------------- 引脚定义 (与 SDK 一致) --------------------

#define IIC_SCL    P13
#define IIC_SDA    P14
#define READ_OUT   (P14F = 0xA2)
#define READ_IN    (P14F = 0xA2)
#define READ_SDA   P14

// 时钟调节 (SDK 默认 1, SSD1306 需 ≤400kHz, 增大以确保稳定)
#define IIC_CLK_ADJ  1

// -------------------- 软件延时 (与 SDK 一致) --------------------

static void delay_us(int i)
{
    long j;
    while (i--)
        j++;
}

// -------------------- 总线初始化 --------------------

static void IIC_Init(void)
{
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us(2);
}

// -------------------- 起始信号 --------------------

static void IIC_Start(void)
{
    IIC_SDA = 1;          // 释放 SDA
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SDA = 0;          // START: SCL 高时 SDA 下降沿
    delay_us(2);
    IIC_SCL = 0;          // 钳住总线，准备发送数据
}

// -------------------- 停止信号 --------------------

static void IIC_Stop(void)
{
    IIC_SCL = 0;
    delay_us(2);
    IIC_SDA = 0;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SDA = 1;          // STOP: SCL 高时 SDA 上升沿
    delay_us(2);
}

// -------------------- 发送一个字节 --------------------

static void IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;

    IIC_SCL = 0;          // 拉低时钟开始传输
    for (t = 0; t < 8; t++)
    {
        if (txd & 0x80)
            IIC_SDA = 1;
        else
            IIC_SDA = 0;

        txd <<= 1;
        delay_us(IIC_CLK_ADJ);
        IIC_SCL = 1;
        delay_us(IIC_CLK_ADJ);
        IIC_SCL = 0;
    }
}

// -------------------- 等待 ACK (与 SDK 完整一致) --------------------

/**
 * 返回值: 0=收到 ACK, 1=超时/无响应
 */
static uint8_t IIC_Wait_Ack(void)
{
    uint16_t ucErrTime = 0;

    IIC_SDA = 1;          // 释放 SDA
    READ_IN;              // SDA 切换为输入
    delay_us(IIC_CLK_ADJ);
    delay_us(IIC_CLK_ADJ);

    IIC_SCL = 1;          // 第 9 个时钟
    delay_us(IIC_CLK_ADJ);

    while (READ_SDA == 1) // 等待从机拉低 SDA (ACK)
    {
        ucErrTime++;
        if (ucErrTime > 2500)
        {
            IIC_SCL = 0;
            IIC_SDA = 1;
            READ_OUT;
            IIC_Stop();
            return 1;     // 超时, 无 ACK
        }
    }

    IIC_SCL = 0;
    IIC_SDA = 1;
    READ_OUT;
    return 0;             // 收到 ACK
}

// -------------------- 公开 API --------------------

void OLED_I2C_Init(void)
{
    // P13 = SCL (推挽输出 + 数字输入)
    P13F = 0xA2;
    // P14 = SDA (与 SDK 一致, 支持输入/输出切换)
    P14F = 0xA2;

    READ_OUT;
    IIC_Init();
}

/**
 * 发送 I2C 数据帧: [ctrl_byte] + [data...]
 *
 * 专为 SSD1306 设计:
 *   ctrl_byte = 0x00 → 命令
 *   ctrl_byte = 0x40 → 显示数据
 *
 * @return 0=成功, 1=NAK
 */
uint8_t OLED_I2C_Send(uint8_t device_addr, uint8_t ctrl_byte,
                      const uint8_t *buf, uint8_t len)
{
    uint8_t i;

    IIC_Start();

    // 设备地址
    IIC_Send_Byte(device_addr);
    if (IIC_Wait_Ack()) {
        return 1;
    }

    // 控制字节
    IIC_Send_Byte(ctrl_byte);
    if (IIC_Wait_Ack()) {
        return 1;
    }

    // 数据字节
    for (i = 0; i < len; i++) {
        IIC_Send_Byte(buf[i]);
        if (IIC_Wait_Ack()) {
            return 1;
        }
    }

    IIC_Stop();
    return 0;
}

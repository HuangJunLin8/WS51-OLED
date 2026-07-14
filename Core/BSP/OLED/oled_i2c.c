/**
 * oled_i2c.c — SSD1306 软件 I2C 驱动实现
 *
 * GPIO 模拟 I2C 主机, ~100kHz @ 16MHz:
 *   P13 = SCL (推挽输出)
 *   P14 = SDA (开漏输出, 需外部 4.7kΩ 上拉)
 */

#include "oled_i2c.h"

// -------------------- 引脚宏 --------------------

#define I2C_SCL_HIGH()   (P13 = 1)
#define I2C_SCL_LOW()    (P13 = 0)
#define I2C_SDA_HIGH()   (P14 = 1)  // 开漏: 写1释放总线
#define I2C_SDA_LOW()    (P14 = 0)  // 开漏: 写0拉低
#define I2C_SDA_READ()   (P14)      // 读 SDA 电平

// -------------------- 软件延时 (~5us @16MHz/12T) --------------------

static void i2c_delay(void)
{
    // 每个 nop = 0.75us, 5us ≈ 7 nop
    // 考虑函数调用开销, 约 5us
    unsigned char i;
    for (i = 0; i < 3; i++) {
        // ~2us per loop iteration
    }
}

// -------------------- I2C 总线操作 --------------------

static void i2c_start(void)
{
    I2C_SDA_HIGH();
    i2c_delay();
    I2C_SCL_HIGH();
    i2c_delay();
    I2C_SDA_LOW();   // SDA 下降沿 → START
    i2c_delay();
    I2C_SCL_LOW();
    i2c_delay();
}

static void i2c_stop(void)
{
    I2C_SDA_LOW();
    i2c_delay();
    I2C_SCL_HIGH();
    i2c_delay();
    I2C_SDA_HIGH();  // SDA 上升沿 → STOP
    i2c_delay();
}

/**
 * 发送一个字节, 返回 ACK (0) 或 NAK (1)
 */
static uint8_t i2c_write_byte(uint8_t dat)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0; i < 8; i++) {
        if (dat & 0x80) {
            I2C_SDA_HIGH();
        } else {
            I2C_SDA_LOW();
        }
        i2c_delay();
        I2C_SCL_HIGH();     // 数据在 SCL 高电平时采样
        i2c_delay();
        I2C_SCL_LOW();
        i2c_delay();
        dat <<= 1;
    }

    // 第 9 个时钟: 读取 ACK
    I2C_SDA_HIGH();         // 释放 SDA, 等待从机拉低
    i2c_delay();
    I2C_SCL_HIGH();
    i2c_delay();
    ack = I2C_SDA_READ();   // 0=ACK, 1=NAK
    I2C_SCL_LOW();
    i2c_delay();

    return ack;
}

// -------------------- 公开 API --------------------

/**
 * 初始化软件 I2C 引脚
 */
void OLED_I2C_Init(void)
{
    // P13 = SCL (推挽输出 + 数字输入)
    P13F = 0x82;
    // P14 = SDA (开漏输出 + 数字输入, 需外部上拉电阻)
    P14F = 0x81;

    // 初始状态: 总线空闲 (SCL=1, SDA=1)
    I2C_SCL_HIGH();
    I2C_SDA_HIGH();
}

/**
 * 发送 I2C 数据帧: [ctrl_byte] + [data...]
 *
 * 专为 SSD1306 设计:
 *   ctrl_byte = 0x00 → 后续字节为命令
 *   ctrl_byte = 0x40 → 后续字节为显示数据
 *
 * @param device_addr  I2C 从机地址 (已左移1位, 如 0x78)
 * @param ctrl_byte    控制字节
 * @param buf          数据缓冲区
 * @param len          数据长度
 * @return 0=成功, 1=NAK
 */
uint8_t OLED_I2C_Send(uint8_t device_addr, uint8_t ctrl_byte,
                      const uint8_t *buf, uint8_t len)
{
    uint8_t i;

    // START
    i2c_start();

    // 设备地址 (写)
    if (i2c_write_byte(device_addr)) {
        i2c_stop();
        return 1;
    }

    // 控制字节
    if (i2c_write_byte(ctrl_byte)) {
        i2c_stop();
        return 1;
    }

    // 数据字节
    for (i = 0; i < len; i++) {
        if (i2c_write_byte(buf[i])) {
            i2c_stop();
            return 1;
        }
    }

    // STOP
    i2c_stop();

    return 0;
}

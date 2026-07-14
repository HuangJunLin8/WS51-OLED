/**
 * oled_i2c.c — SSD1306 硬件 I2C 驱动实现
 *
 * 参考 SDK "19-IIC-host-硬件"，使用 WS51F6240 硬件 I2C 外设:
 *   P02 = SCL (IIC 复用功能, P02F=0xA5)
 *   P16 = SDA (IIC 复用功能, P16F=0xA5)
 *
 * 注意: SDA 需外部 4.7kΩ 上拉电阻
 */

#include "oled_i2c.h"

// -------------------- 硬件 I2C 初始化 (与 SDK I2C_Init 一致) --------------------

void OLED_I2C_Init(void)
{
    // P02 = SCL, P16 = SDA (IIC 复用功能, 与 SDK 一致)
    P02F = 0xA5;
    P16F = 0xA5;

    // I2C 时钟配置: SCL = Fi2c / (I2CFG1[6:0] + 8)
    // Fi2c = 16MHz, I2CFG1 = 0xFF (127)
    // SCL = 16,000,000 / (127 + 8) ≈ 119kHz
    I2CCON  = 0x00;             // Fi2c_CLK = 16MHz (不分频)
    I2CFG1  = 0xFF;             // 时钟分频器

    I2CFLG  = 0x00;             // 清除所有标志

    I2CCON |= (0x1 << 7);       // bit7 I2CE: 使能 I2C 模块
    I2CCON &= ~(0x1 << 6);      // bit6 I2CIE: 禁能 I2C 中断
    I2CCON &= ~(0x1 << 5);      // bit5 STAIE: 禁能 START 中断
    I2CCON &= ~(0x1 << 4);      // bit4 STPIE: 禁能 STOP 中断
}

// -------------------- 硬件 I2C 发送 (基于 SDK I2C_WriteByteNum) --------------------

/**
 * 发送 I2C 数据帧: [device_addr] + [ctrl_byte] + [data...]
 *
 * 专为 SSD1306 设计:
 *   ctrl_byte = 0x00 → 命令
 *   ctrl_byte = 0x40 → 显示数据
 *
 * @param device_addr  I2C 从机地址 (7-bit 左移1位)
 * @param ctrl_byte    SSD1306 控制字节
 * @param buf          数据缓冲区
 * @param len          数据长度 (字节数)
 * @return 0=成功, 1=仲裁丢失, 2=收到 NACK
 */
uint8_t OLED_I2C_Send(uint8_t device_addr, uint8_t ctrl_byte,
                      const uint8_t *buf, uint8_t len)
{
    uint8_t cnt = 0;
    uint8_t total_len = len + 1;    // ctrl_byte + data 总字节数

    I2CFLG = 0;                     // 清除标志
    I2CTXD = device_addr;           // 装载从机地址
    I2CCON |= (0x1 << 3);           // 发送 START 信号

    while (1)
    {
        // START 信号已发出
        if (IF_RXSTA == (I2CFLG & IF_RXSTA))
        {
            I2CFLG &= ~IF_RXSTA;
        }

        // 一字节发送完成并收到 ACK
        if (IF_TXDAT == (I2CFLG & IF_TXDAT))
        {
            // 加载下一字节: 先发 ctrl_byte, 再发数据
            if (cnt == 0)
                I2CTXD = ctrl_byte;
            else
                I2CTXD = buf[cnt - 1];

            cnt++;

            // 全部字节已加载, 发送 STOP
            if (cnt >= total_len)
            {
                I2CCON |= (0x1 << 2);   // STOP
                I2CFLG &= ~IF_TXDAT;
                I2CFLG = 0;
                return 0;
            }

            I2CFLG &= ~IF_TXDAT;

            // 检查 NACK
            if (RXNAK == (I2CFLG & RXNAK))
            {
                I2CCON |= (0x1 << 2);   // STOP
                I2CFLG = 0;
                return 2;
            }
        }

        // 仲裁丢失
        if (IF_LSTARB == (I2CFLG & IF_LSTARB))
        {
            I2CFLG &= ~IF_LSTARB;
            I2CFLG = 0;
            return 1;
        }
    }
}

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
    // P02 = SCL, P16 = SDA (IIC 复用)
    P02F = 0xA5;
    P16F = 0xA5;

    // I2C 时钟分频选择：00 = 16MHz 内部时钟
    I2CCON = 0x00;

    // SCL 频率 = Fi2c / (I2CFG1[6:0] + 8)
    // 0xFF → 16MHz / (127+8) ≈ 119KHz
    I2CFG1 = 0xFF;

    // 清除所有标志
    I2CFLG = 0x00;

    // 使能 I2C 模块 (bit7 = I2CE)
    I2CCON |= 0x80;

    // 禁止所有 I2C 中断 — 使用轮询方式
    I2CCON &= ~0x40;   // 禁能 I2C 中断 (I2CIE = 0)
    I2CCON &= ~0x20;   // 禁能 START 中断
    I2CCON &= ~0x10;   // 禁能 STOP 中断
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
    uint8_t total_len = len + 1;   // ctrl_byte + data
    uint16_t timeout = 50000;

    // 清标志
    I2CFLG = 0;

    // 地址
    I2CTXD = device_addr;

    // START
    I2CCON |= (1 << 3);

    while (1)
    {

        // 发送完成，收到 ACK
        if (I2CFLG & IF_TXDAT)
        {

            // 检查 NACK
            if (I2CFLG & RXNAK)
            {
                I2CCON |= (1 << 2);   // STOP
                I2CFLG = 0;
                return 2;
            }

            /*
             * 当前字节已经发送完成
             * 决定是否发送下一个字节
             */
            if (cnt < total_len)
            {

                // 第一个数据是控制字节
                if (cnt == 0){
                    I2CTXD = ctrl_byte;
                }
                else{
                    I2CTXD = buf[cnt - 1];
                }

                cnt++;

                /*
                 * 注意：
                 * 最后一个 TXD 写入后，
                 * 不要立即 STOP
                 *
                 * 等下一次 IF_TXDAT，
                 * 表示最后一个字节真正发送完成，
                 * 再 STOP
                 */
            }
            else{
                // 所有数据发送完成
                // 此时发送 STOP
                I2CCON |= (1 << 2);

                timeout = 50000;
                while (!(I2CFLG & BUSIDLE))
                {
                    if (--timeout == 0)
                        break;
                }

                I2CFLG = 0;
                return 0;
            }

            // 清发送完成标志
            I2CFLG &= ~IF_TXDAT;
        }

        // 仲裁丢失
        if (I2CFLG & IF_LSTARB)
        {
            I2CFLG &= ~IF_LSTARB;
            I2CFLG = 0;
            return 1;
        }

        // 超时保护
        if (--timeout == 0)
        {
            I2CCON |= (1 << 2);
            I2CFLG = 0;
            return 3;
        }
    }
}

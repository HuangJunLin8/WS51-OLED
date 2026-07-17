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

// -------------------- 硬件 I2C 发送 --------------------


/**
 * @brief SSD1306 I2C发送
 *
 * 数据格式:
 * START
 * 地址(W)
 * ctrl_byte
 * DATA...
 * STOP
 *
 * ctrl:
 * 0x00 命令
 * 0x40 显存数据
 *
 * 返回:
 * 0 成功
 * 1 仲裁丢失
 * 2 NACK
 * 3 超时
 */
unsigned char OLED_I2C_Send(unsigned char device_addr,
                            unsigned char ctrl_byte,
                            unsigned char *buf,
                            unsigned char len)
{
    unsigned char cnt = 0;
    unsigned int timeout = 50000;


    I2CFLG = 0;


    // 地址
    I2CTXD = device_addr;


    // START
    I2CCON |= (1 << 3);


    while(1)
    {
        if(I2CFLG & IF_TXDAT)
        {

            // ACK检查
            if(I2CFLG & RXNAK)
            {
                I2CCON |= (1<<2);
                I2CFLG = 0;
                return 2;
            }


            /*
             * 第一次发送控制字节
             * 后续发送buf[]
             */
            if(cnt == 0)
            {
                I2CTXD = ctrl_byte;
                cnt++;
            }
            else if(cnt <= len)
            {
                I2CTXD = buf[cnt - 1];
                cnt++;
            }


            /*
             * 所有数据装载完成
             */
            if(cnt > len)
            {
                // STOP
                I2CCON |= (1<<2);


                // 清标志
                I2CFLG &= ~IF_TXDAT;


                timeout = 50000;
                while(!(I2CFLG & BUSIDLE))
                {
                    if(--timeout == 0)
                        break;
                }


                I2CFLG = 0;
                return 0;
            }


            I2CFLG &= ~IF_TXDAT;
        }


        if(I2CFLG & IF_LSTARB)
        {
            I2CFLG &= ~IF_LSTARB;
            return 1;
        }


        if(--timeout == 0)
        {
            I2CCON |= (1<<2);
            I2CFLG = 0;
            return 3;
        }
    }
}
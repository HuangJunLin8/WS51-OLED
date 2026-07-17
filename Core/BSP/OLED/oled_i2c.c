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


/*
 * I2C 发送一次 START + 设备地址 + 数据块 + STOP
 *
 * 时序（逻辑分析仪可观察）：
 *   START → DevAddr(W) → ACK → Data[0] → ACK → ... → Data[N-1] → ACK → STOP
 *
 * 返回 0 = 成功， 2 = NACK，3 = 超时
 */
unsigned char I2C_SendBurst(unsigned char dev_addr,
                            unsigned char *buf,
                            unsigned char len)
{
    unsigned char cnt = 0;
    unsigned int timeout = 50000;


    I2CFLG = 0;

    // 发送地址
    I2CTXD = dev_addr;

    // START
    I2CCON |= (1 << 3);


    while(1)
    {
        if(I2CFLG & IF_TXDAT)
        {

            // 检查 ACK
            if(I2CFLG & RXNAK)
            {
                I2CCON |= (1<<2);
                I2CFLG = 0;
                return 2;
            }

            // 还有数据
            if(cnt < len)
            {
                I2CTXD = buf[cnt++];
            }

            // 所有数据已经装载
            if(cnt >= len)
            {
                // 注意顺序：
                // STOP 必须在清 TXDAT 前
                I2CCON |= (1<<2);

                // 再清标志
                I2CFLG &= ~IF_TXDAT;


                timeout=50000;
                while(!(I2CFLG & BUSIDLE))
                {
                    if(--timeout==0)
                        break;
                }

                I2CFLG=0;
                return 0;
            }

            I2CFLG &= ~IF_TXDAT;
        }

        if(I2CFLG & IF_LSTARB)
        {
            I2CFLG &= ~IF_LSTARB;
            return 1;
        }

        if(--timeout==0)
        {
            I2CCON |= (1<<2);
            I2CFLG=0;
            return 3;
        }
    }
}



/**
 * @brief SSD1306 I2C发送
 *
 * 数据格式:
 * START
 * 地址
 * 控制字节
 * DATA...
 * STOP
 *
 * ctrl:
 * 0x00 命令
 * 0x40 显存数据
 */
unsigned char OLED_I2C_Send(unsigned char device_addr,
                      unsigned char ctrl_byte,
                      const unsigned char *buf,
                      unsigned char len)
{
    data unsigned char tx_buf[32];     // 根据一次发送最大长度调整
    data unsigned char i;


    // 第一个字节是SSD1306控制字节
    tx_buf[0] = ctrl_byte;


    // 后面跟实际数据
    for(i = 0; i < len; i++)
    {
        tx_buf[i + 1] = buf[i];
    }


    // 直接使用已经验证成功的I2C发送函数
    return I2C_SendBurst(device_addr,
                         tx_buf,
                         len + 1);
}
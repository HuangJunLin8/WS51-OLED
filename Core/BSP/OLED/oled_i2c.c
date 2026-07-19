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

// -------------------- 硬件 I2C 初始化 --------------------

void OLED_I2C_Init(void)
{
    // P13 = SCL, P14 = SDA
    P13F = 0x65;
    P14F = 0x65;

    // I2C 时钟分频选择：
    // 00 = 16MHz
    // 01 = 16Mhz / 2 = 8Mhz
    // 02 = 16Mhz / 3 = 5.33Mhz
    // 03 = 16Mhz / 4 = 4Mhz

    // 实测：
    // 16Mhz SCL 低电平间隔 1~2us (逻辑分析仪 20Mhz 分析有时不准)
    // 4Mhz  SCL 低电平间隔 10us  (逻辑分析仪 20Mhz 分析准确)
    
    I2CCON = 0x02;

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

// ------------------------------- 硬件 I2C 发送 -------------------------------

 /*
 * I2C 发送一次 START + 设备地址 + 控制字 +数据块 + STOP
 *
 * 时序：
 *   START → DevAddr(W) → ACK → Ctrl → ACK → Data[0] → ACK → ... → Data[N-1] → ACK → STOP
 *
 * 返回 0: 成功， 2: NACK，3: 超时
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

            // 检查 ACK
            // if(I2CFLG & RXNAK)
            // {
            //    I2CCON |= (1<<2);
            //    I2CFLG = 0;
            //    return 2;
            // }


            // 发送控制字节
            if(cnt == 0)
            {
                            
                // 先清 TXDAT 标志
                I2CFLG &= ~IF_TXDAT;

                // 再压入新数据
                I2CTXD = ctrl_byte;
                cnt++;
            }
                        
            // 发送数据
            else if(cnt <= len)
            {
                            
                // 先清 TXDAT 标志
                I2CFLG &= ~IF_TXDAT;

                // 再压入新数据
                I2CTXD = buf[cnt - 1];
                              cnt++;
            }


            // 所有数据装载完成
            if(cnt > len)
            {
                // 先发 STOP
                I2CCON |= (1<<2);

                // 再清全部标志
                I2CFLG = 0;

                timeout = 50000;
                while(!(I2CFLG & BUSIDLE))
                {
                    if(--timeout == 0)
                        break;
                }

                return 0;
            }

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




/*
 * I2C 发送一次 START + 设备地址 + 数据块 + STOP
 *
 * 时序：
 *   START → DevAddr(W) → ACK → Data[0] → ACK → ... → Data[N-1] → ACK → STOP
 *
 * 返回 0: 成功， 2: NACK，3: 超时
 */
unsigned char I2C_SendBurst(unsigned char dev_addr,
                            unsigned char *buf,
                            unsigned char len)
{
    unsigned char cnt = 0;
    unsigned int timeout = 50000;

    // 清全部标志
    I2CFLG = 0;

    // 压入地址
    I2CTXD = dev_addr;

    // 启动发送
    I2CCON |= (1 << 3);

    // 循环发送数据
    while(1)
    {
        if(I2CFLG & IF_TXDAT)
        {

            // 检查 ACK
            // if(I2CFLG & RXNAK)
            // {
            //    I2CCON |= (1<<2);
            //    I2CFLG = 0;
            //    return 2;
            // }
                        

            // 还有数据
            if(cnt < len)
            {
                            
                // 先清 TXDAT 标志
                I2CFLG &= ~IF_TXDAT;

                // 再压入新数据
                I2CTXD = buf[cnt++];
            }


            // 所有数据已经装载
            if(cnt >= len)
            {
                // 注意顺序：
                // STOP 必须在清 TXDAT 前
                I2CCON |= (1<<2);

                // 再清全部标志
                I2CFLG=0;

                timeout=50000;
                while(!(I2CFLG & BUSIDLE))
                {
                    if(--timeout==0)
                        break;
                }

                return 0;
            }
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
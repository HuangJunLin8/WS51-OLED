/**
 * oled_i2c.c — SSD1306 硬件 IIC 驱动实现
 *
 * 基于 SDK 19-IIC-host-硬件 示例改写，适配 SSD1306 OLED。
 * 使用轮询模式（非中断），简化 8051 移植。
 */

#include "oled_i2c.h"

/**
 * 初始化硬件 IIC 模块
 *
 * - P02 配置为 HW IIC SCL  (0xA5)
 * - P16 配置为 HW IIC SDA  (0xA5)
 * - 时钟: Fi2c = 16MHz / (127+8) ≈ 119kHz
 * - 轮询模式（关闭 I2C 中断）
 */
void OLED_I2C_Init(void)
{
    /* 配置硬件 IIC 引脚 */
    P02F = 0xA5;    /* P02 = HW IIC SCL */
    P16F = 0xA5;    /* P16 = HW IIC SDA */

    /* I2C 时钟配置 */
    I2CCON = 0x00;          /* Fi2c_CLK = 16MHz, 不分频 */
    I2CFG1 = 0xff;          /* 波特率 = 16MHz / (127+8) ≈ 119kHz */
    I2CFLG = 0x00;          /* 清除所有标志位 */

    /* 使能 I2C 模块 (轮询模式，关闭中断) */
    I2CCON |= (1 << 7);     /* I2CEN = 1, 使能 I2C */
    I2CCON &= ~(1 << 6);    /* I2CIE = 0, 关闭中断 */
    I2CCON &= ~(1 << 5);    /* STAIE = 0, 关闭 START 中断 */
    I2CCON &= ~(1 << 4);    /* STPIE = 0, 关闭 STOP 中断 */
}

/**
 * 发送 I2C 数据: [ctrl_byte] + [data...]
 *
 * 专为 SSD1306 设计:
 *   ctrl_byte = 0x00 → 后续字节为命令
 *   ctrl_byte = 0x40 → 后续字节为显示数据
 *
 * @param device_addr  I2C 从机地址 (已左移1位, 如 0x78)
 * @param ctrl_byte    控制字节 (0x00=命令, 0x40=数据)
 * @param data         数据缓冲区指针
 * @param len          数据长度
 * @return 0=成功, 1=仲裁丢失, 2=收到 NAK
 */
uint8_t OLED_I2C_Send(uint8_t device_addr, uint8_t ctrl_byte,
                      const uint8_t *buf, uint8_t len)
{
    uint8_t i, ret;

    I2CFLG = 0;                     /* 清除所有标志 */

    /* 发送 START + 设备地址 */
    I2CTXD = device_addr;
    I2CCON |= (1 << 3);             /* STA = 1, 产生 START */

    /* ========== 发送控制字节 ========== */
    while (1)
    {
        if (I2CFLG & IF_TXDAT)      /* 上一字节已发送并收到 ACK */
        {
            I2CFLG &= ~IF_TXDAT;
            I2CTXD = ctrl_byte;     /* 发送控制字节 */
            break;
        }
        if (I2CFLG & RXNAK)         /* 收到 NAK */
        {
            ret = 2;
            goto i2c_stop;
        }
        if (I2CFLG & IF_LSTARB)     /* 仲裁丢失 */
        {
            ret = 1;
            goto i2c_stop;
        }
    }

    /* ========== 发送数据字节 ========== */
    i = 0;
    while (1)
    {
        if (I2CFLG & IF_TXDAT)
        {
            I2CFLG &= ~IF_TXDAT;

            if (i >= len)
                break;              /* 所有数据已发送完毕 */

            I2CTXD = buf[i++];       /* 发送下一个数据字节 */
        }
        if (I2CFLG & RXNAK)
        {
            ret = 2;
            goto i2c_stop;
        }
        if (I2CFLG & IF_LSTARB)
        {
            ret = 1;
            goto i2c_stop;
        }
    }

    /* 等待最后一个字节发送完成 */
    while (!(I2CFLG & IF_TXDAT));
    I2CFLG &= ~IF_TXDAT;
    ret = 0;

i2c_stop:
    I2CCON |= (1 << 2);             /* STP = 1, 产生 STOP */
    I2CFLG  = 0;                    /* 清除标志 */
    return ret;
}

/**
 * oled_i2c.h — SSD1306 硬件 I2C 驱动 (WS51F6240)
 *
 * 参考 SDK "19-IIC-host-硬件"，使用硬件 I2C 外设:
 *   - P02 = SCL (IIC 复用功能, 0xA5)
 *   - P16 = SDA (IIC 复用功能, 0xA5)
 *   - SCL 频率 ~119kHz @ 16MHz HRC
 *   - 轮询模式 (无 I2C 中断)
 */

#ifndef OLED_I2C_H
#define OLED_I2C_H

#include "WS51F6240.h"
#include "main.h"

// SSD1306 I2C 从机地址 (7-bit: 0x3C, 左移1位 = 0x78)
#define OLED_I2C_ADDR   0x78

// -------------------- I2C 标志位 (I2CFLG, 与 SDK 一致) --------------------

#define BUSIDLE     0x80    // 总线空闲 (只读)
#define RXNAK       0x40    // 收到 NACK (只读)
#define IF_LSTARB   0x20    // 仲裁丢失 (写0清除)
#define IF_RXSTA    0x10    // 检测到 START 信号
#define IF_RXSTP    0x08    // 检测到 STOP 信号
#define IF_TXDAT    0x04    // 主机模式: 一字节发送完成并收到 ACK
#define IF_RXDAT    0x02    // 收到一字节数据
#define IF_RXADR    0x01    // 从机模式: 正确收到从机地址

// -------------------- 函数声明 --------------------

void    OLED_I2C_Init(void);

unsigned char OLED_I2C_Send(unsigned char device_addr,
                            unsigned char ctrl_byte,
                            unsigned char *buf,
                            unsigned char len);
											
unsigned char I2C_SendBurst(unsigned char dev_addr, unsigned char *buf, unsigned char len);

#endif // OLED_I2C_H

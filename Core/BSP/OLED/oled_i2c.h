/**
 * oled_i2c.h — SSD1306 硬件 IIC 驱动 (WS51F6240)
 *
 * 使用 WS51F6240 硬件 IIC 模块:
 *   - P02 = SCL (固定硬件 IIC 引脚)
 *   - P16 = SDA (固定硬件 IIC 引脚，也是调试/下载接口)
 *   - 时钟频率 ~119kHz @ 16MHz HRC
 *
 * 注意: P02/P16 也是 I2C 调试下载接口，上电有 64ms 延时。
 *       如屏蔽此延时，需在 main() 开头加延时，否则无法再次下载。
 */

#ifndef OLED_I2C_H
#define OLED_I2C_H

#include "WS51F6240.h"

/* SSD1306 I2C 从机地址 (7-bit: 0x3C, 左移1位 = 0x78) */
#define OLED_I2C_ADDR   0x78

/* I2CFLG 标志位 (与 SDK 一致) */
#define BUSIDLE    0x80
#define RXNAK      0x40
#define IF_LSTARB  0x20
#define IF_RXSTA   0x10
#define IF_RXSTP   0x08
#define IF_TXDAT   0x04
#define IF_RXDAT   0x02
#define IF_RXADR   0x01

/* ==================== 函数声明 ==================== */

void OLED_I2C_Init(void);
uint8_t OLED_I2C_Send(uint8_t device_addr, uint8_t ctrl_byte,
                      const uint8_t *buf, uint8_t len);

#endif /* OLED_I2C_H */

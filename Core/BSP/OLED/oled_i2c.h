/**
 * oled_i2c.h — SSD1306 软件 I2C 驱动 (WS51F6240)
 *
 * 使用 GPIO 模拟 I2C:
 *   - P13 = SCL (推挽输出)
 *   - P14 = SDA (推挽输出/输入切换)
 *   - 时钟频率 ~100kHz @ 16MHz HRC
 */

#ifndef OLED_I2C_H
#define OLED_I2C_H

#include "WS51F6240.h"

// SSD1306 I2C 从机地址 (7-bit: 0x3C, 左移1位 = 0x78)
#define OLED_I2C_ADDR   0x78

// -------------------- 函数声明 --------------------

void    OLED_I2C_Init(void);
uint8_t OLED_I2C_Send(uint8_t device_addr, uint8_t ctrl_byte,
                      const uint8_t *buf, uint8_t len);

#endif // OLED_I2C_H

/**
 * oled_disp.h — SSD1306 96x16 OLED 显示驱动
 *
 * 提供:
 *   - 192 字节帧缓冲区 (2 页 × 96 列)
 *   - 像素级绘制 API
 *   - 字体字符串绘制 (通过 font.c 解码器)
 */

#ifndef OLED_DISP_H
#define OLED_DISP_H

#include "stdint.h"
#include "font.h"
#include "main.h"

// 显示尺寸
#define OLED_WIDTH   96
#define OLED_HEIGHT  16
#define OLED_PAGES   2 // HEIGHT / 8

// -------------------- 函数声明 --------------------

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Flush(void);
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t on);
void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t len, uint8_t on);
void OLED_DrawString(uint8_t x, uint8_t y, const char *str);
void OLED_DrawString_F(uint8_t x, uint8_t y, const char *str, font_t *f);

// 暴露帧缓冲区指针 (供 font.c 回调使用)
extern uint8_t xdata g_oled_fb[192];

#endif // OLED_DISP_H

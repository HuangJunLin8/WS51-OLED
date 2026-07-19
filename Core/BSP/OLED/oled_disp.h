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
#define OLED_PAGES   2 


// 暴露帧缓冲区指针 (供 font.c 回调使用)
extern uint8_t xdata g_oled_fb[192];



// ------------------------------ 控制函数 ---------------------------------

// 初始化
void OLED_Init(void);

// 刷屏
void OLED_Flush(void);

// 清屏
void OLED_Clear(void);



// ---------------------------- 基本绘图函数 -------------------------------

// 绘制单像素
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t on);

// 绘制水平线
void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t len, uint8_t on);

// 绘制垂直线段
void OLED_DrawVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t on);

// 绘制填充矩形
void OLED_FillRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t on);

// 绘制一像素矩形框
void OLED_DrawFrame(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t on);


// ---------------------------- 高级绘图函数 -------------------------------

// 绘制任意角度线段 (Bresenham)
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t on);

// 绘制填充圆角矩形
void OLED_DrawRoundedBox(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t r,uint8_t on);

// 绘制圆角矩形框
void OLED_DrawRoundedFrame(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t r,uint8_t on);

// 设置全局字体
void OLED_SetFont(const uint8_t *font_buf);

// 绘制英文文字
void OLED_DrawASCII(uint8_t x, uint8_t y, const char *str);

// 绘制中文文字
void OLED_DrawUTF8(uint8_t x, uint8_t y, const char *str);

#endif // OLED_DISP_H

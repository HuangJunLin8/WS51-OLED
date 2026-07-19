/**
 * main.c — SSD1306 96x16 OLED 软件 IIC 测试 (WS51F6240)
 *
 * 硬件连接:
 *   P13 = SCL (软件 I2C)
 *   P14 = SDA (软件 I2C, 外部 4.7kΩ 上拉)
 *
 * 测试: 全屏矩形框闪烁
 */

#include "WS51F6240.h"
#include "oled_disp.h"

// -------------------- 全局变量 --------------------

volatile unsigned long g_tick_ms = 0;

// -------------------- 定时器 & 延时 --------------------

void init_timer0(void)
{
    TL0 = (65536 - 1334);
    TH0 = (65536 - 1334) >> 8;
    TMOD = 0x01;
    ET0 = 1;
    EA  = 1;
    TR0 = 1;
}

void timer0_isr(void) interrupt 1
{
    TL0 = (65536 - 1334);
    TH0 = (65536 - 1334) >> 8;
    g_tick_ms++;
}

void delay_ms(unsigned int ms)
{
    unsigned long target = g_tick_ms + ms;
    while (g_tick_ms < target);
}

// -------------------- 绘制矩形框 --------------------

static void draw_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    uint8_t i;

    // 上边
    for (i = x1; i <= x2; i++) OLED_SetPixel(i, y1, 1);
    // 下边
    for (i = x1; i <= x2; i++) OLED_SetPixel(i, y2, 1);
    // 左边
    for (i = y1; i <= y2; i++) OLED_SetPixel(x1, i, 1);
    // 右边
    for (i = y1; i <= y2; i++) OLED_SetPixel(x2, i, 1);
}

// -------------------- 主函数 --------------------

void main()
{
    // 系统时钟: HRC 16MHz
    SCCON  = 0x00;
    HRCON |= 0x80;

    init_timer0();
    delay_ms(1000);          // 上电稳定

    OLED_I2C_Init();
    delay_ms(100);          // 待 I2C 初始化
  
    OLED_Init();
    
    while (1)
    {
        // 测试单像素
        OLED_Clear();
        OLED_SetPixel(48,8,1);
        OLED_Flush();
        delay_ms(500);


        // 测试水平线
        OLED_Clear();
        OLED_DrawHLine(24,5,50,1);
        OLED_Flush();
        delay_ms(500);


        // 测试垂直线
        OLED_Clear();
        OLED_DrawVLine(48,2,12,1);
        OLED_Flush();
        delay_ms(500);


        // 测试斜线
        OLED_Clear();
        OLED_DrawLine(0,0,95,15,1);
        OLED_Flush();
        delay_ms(500);


        // 测试填充矩形
        OLED_Clear();
        OLED_FillRect(20,3,30,10,1);
        OLED_Flush();
        delay_ms(500);


        // 测试矩形框
        OLED_Clear();
        OLED_DrawFrame(10,2,50,12,1);
        OLED_Flush();
        delay_ms(500);


        // 测试圆角矩形
        OLED_Clear();
        OLED_DrawRoundedBox(20,2,50,12,3,1);
        OLED_Flush();
        delay_ms(500);


        // 测试圆角矩形框
        OLED_Clear();
        OLED_DrawRoundedFrame(20,2,50,12,3,1);
        OLED_Flush();
        delay_ms(500);


        // 测试ASCII字符串
        OLED_Clear();
        OLED_DrawASCII(5,1,"WS51F6240");
        OLED_Flush();
        delay_ms(500);
    }
}

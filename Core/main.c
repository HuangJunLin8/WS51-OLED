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
    delay_ms(100);          // 上电稳定

    OLED_Init();            // 软件 I2C + SSD1306 初始化

    while (1)
    {
        // 绘制全屏矩形框
        OLED_Clear();
        draw_rect(0, 0, OLED_WIDTH - 1, OLED_HEIGHT - 1);
			
				OLED_DrawString(10, 3, "WS51F6240");
				
        OLED_Flush();
        delay_ms(500);

        // 清屏
        OLED_Clear();
        OLED_Flush();
        delay_ms(500);
    }
}

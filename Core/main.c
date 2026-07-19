/**
 * main.c — SSD1306 96x16 OLED 硬件 IIC 测试 (WS51F6240)
 *
 * 参考 SDK "19-IIC-host-硬件":
 *   P02 = SCL (硬件 I2C)
 *   P16 = SDA (硬件 I2C, 外部 4.7kΩ 上拉)
 *
 * 测试: 全屏矩形框闪烁
 */

#include "WS51F6240.h"
#include "oled_disp.h"
#include "oled_i2c.h"
#include "main.h"

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


// us 延时(1~49000us)
void delay_us(unsigned int us)
{
    unsigned char  th, tl;
    unsigned int   start, now, elapsed;
    unsigned int   ticks;

    if (us == 0) return;

    // 目标 tick 数: us * 4 / 3 (每 tick = 0.75us)
    ticks = us + us / 3;

    // 关闭 Timer0 中断，防止 ISR 在测量期间重置计数值
    ET0 = 0;

    do {
        th = TH0;
        tl = TL0;
    } while (th != TH0);
    start = (th << 8) | tl;

    // 轮询计数值直到达到目标 tick 数
    do {
        do {
            th = TH0;
            tl = TL0;
        } while (th != TH0);
        now = (th << 8) | tl;

        // 处理 16-bit 计数器溢出
        if (now >= start)
            elapsed = now - start;
        else
            elapsed = (65536 - start) + now;
    } while (elapsed < ticks);

    // 恢复 Timer0 中断
    ET0 = 1;
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
    delay_ms(200);          // 上电稳定
    
    OLED_I2C_Init();
    delay_ms(100);          // 无此延时，IIC 时序会乱
        

    while (1)
    {
        OLED_Init();

        OLED_Clear();
        draw_rect(0, 0, OLED_WIDTH - 1, OLED_HEIGHT - 1);
        OLED_DrawString(10, 3, "WS51F6240");
            
        OLED_Flush();
        delay_ms(500);
    }
}

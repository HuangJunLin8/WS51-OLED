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


void OLED_TestFillLine(void)
{
    uint8_t x;
    uint8_t y;

    OLED_Flush();

    while(1)
    {
        // 逐行填充白色
        for(y = 0; y < 16; y++)
        {
            for(x = 0; x < 96; x++)
            {
                OLED_SetPixel(x, y, 1);
            }
            OLED_Flush();
        }


        // 逐行填充黑色
        for(y = 0; y < 16; y++)
        {
            for(x = 0; x < 96; x++)
            {
                OLED_SetPixel(x, y, 0);
            }
            OLED_Flush();
        }
    }
}


// -------------------- 主函数 --------------------
void main()
{
    SCCON  = 0x00;          // 系统时钟: HRC 16MHz
    HRCON |= 0x80;

    init_timer0();
    delay_ms(200);          // 上电稳定
    
    OLED_I2C_Init();
    delay_ms(100);          // 待 I2C 初始化

    OLED_Init();

    OLED_TestFillLine();

    while(1)
    {
         
    }
}

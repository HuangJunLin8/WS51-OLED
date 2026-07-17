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




// -------------------- 主函数 --------------------

void main()
{
	  unsigned char data2[8]={0x00,0xAA,0x00,0xAA,0x00,0xAA,0x00,0xAA};
	
    // 系统时钟: HRC 16MHz
    SCCON  = 0x00;
    HRCON |= 0x80;

    init_timer0();
    delay_ms(200);          // 上电稳定
	
		OLED_I2C_Init();
		delay_ms(100);          // 无此延时，时序会乱
		

    while (1)
    {
			
				OLED_Init();

        delay_ms(2000);
			
			  // OLED_Clear();
			  // OLED_Flush();
			
			  
			  // delay_ms(2000);
			
    }
}

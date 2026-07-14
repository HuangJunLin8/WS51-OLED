/**
 * main.c — SSD1306 96x16 OLED 软件 IIC 演示 (WS51F6240)
 *
 * 硬件连接:
 *   P13 = SCL (软件 I2C, 推挽输出)
 *   P14 = SDA (软件 I2C, 开漏 + 外部 4.7kΩ 上拉)
 *
 * 功能: 在 0.69" OLED 上演示文字绘制
 */

#include "WS51F6240.h"
#include "oled_disp.h"

// -------------------- 全局变量 --------------------

// 毫秒计数器，在 Timer0 中断中递增
volatile unsigned long g_tick_ms = 0;



// -------------------- 定时器 & 延时 --------------------

/**
 * 初始化 Timer0
 * Mode 1 (16-bit): 1ms 定时 @16MHz/12T
 * 计数 = 16MHz / 12 * 0.001 = 1334
 * 重载值 = 65536 - 1334 = 64202
 */
void init_timer0(void)
{
    TL0 = (65536 - 1334); // 初始值低字节
    TH0 = (65536 - 1334) >> 8; // 初始值高字节
    TMOD = 0x01; // Timer0 Mode 1, 12T 分频

    ET0 = 1; // 使能 Timer0 中断
    EA  = 1; // 使能总中断
    TR0 = 1; // 启动 Timer0
}

/**
 * Timer0 中断服务函数 (interrupt 1)
 * 每 1ms 重装初值并更新毫秒计数器
 */
void timer0_isr(void) interrupt 1
{
    TL0 = (65536 - 1334);
    TH0 = (65536 - 1334) >> 8;
    g_tick_ms++;
}

/**
 * 毫秒延时
 */
void delay_ms(unsigned int ms)
{
    unsigned long target;
    target = g_tick_ms + ms;
    while (g_tick_ms < target);
}




// -------------------- 演示函数 --------------------

/**
 * 绘制一个简单的动画帧: 水平移动的像素条
 */
static void demo_scroll_bar(uint8_t pos)
{
    uint8_t x;

    OLED_Clear();

    // 第 0 行 (y=0-7): 移动的方块
    for (x = pos; x < pos + 8 && x < OLED_WIDTH; x++) {
        OLED_SetPixel(x, 0, 1);
        OLED_SetPixel(x, 1, 1);
        OLED_SetPixel(x, 2, 1);
    }

    OLED_Flush();
}

/**
 * 绘制文字信息
 */
static void demo_text(uint8_t page)
{
    OLED_Clear();

    switch (page) {
    case 0:
        OLED_DrawString(0, 1, "WS51F6240");
        break;
    case 1:
        OLED_DrawString(0, 1, "OLED 96x16");
        break;
    case 2:
        OLED_DrawString(0, 1, "HW IIC OK!");
        break;
    default:
        OLED_DrawString(0, 1, "Hello!");
        break;
    }

    OLED_Flush();
}




// -------------------- 主函数 --------------------

void main()
{
    uint8_t  demo_idx;
    uint8_t  bar_pos;
    uint16_t wait;

    // 系统时钟: HRC 16MHz
    SCCON  = 0x00;
    HRCON |= 0x80;

    // 初始化 Timer0 (1ms tick)
    init_timer0();

	  // 至少 68ms 延时防止烧录检测和 OLED IIC 冲突
	  delay_ms(100); 
	
    /*
     * 初始化 OLED
     * 内部配置 P02=SCL, P16=SDA (硬件 IIC)
     * 发送 SSD1306 初始化命令序列
     */
    OLED_Init();

    // 开机显示
    OLED_Clear();
    OLED_DrawString(5, 1, "WS51F6240");
    OLED_Flush();
    delay_ms(1500);

    // -------------------- 主循环: 交替显示文字和动画 --------------------
    demo_idx = 0;
    bar_pos  = 0;

    while (1)
    {
        // -------------------- 阶段 1: 显示文字 (每种文字显示 2 秒) --------------------
        demo_text(demo_idx);
        delay_ms(2000);

        demo_idx++;
        if (demo_idx >= 4)
            demo_idx = 0;

        // -------------------- 阶段 2: 滚动条动画 (约 3 秒) --------------------
        for (bar_pos = 0; bar_pos < OLED_WIDTH - 8; bar_pos++) {
            demo_scroll_bar(bar_pos);
            delay_ms(30);
        }
        for (bar_pos = OLED_WIDTH - 8; bar_pos > 0; bar_pos--) {
            demo_scroll_bar(bar_pos);
            delay_ms(30);
        }

        // 简单的防 watchdog 延时
        for (wait = 0; wait < 1000; wait++) {
            // nop
        }
    }
}

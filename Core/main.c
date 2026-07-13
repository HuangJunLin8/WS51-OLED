#include "WS51F6240.h"


// 毫秒计数器，在中断中递增
volatile unsigned long g_tick_ms = 0;


// ------------------ 定时器初始化 ------------------

/*
 * 定时时间 = ((重载值 - 初始值) * 12 * 1000 * 1000) / 系统频率
 * 初始值   = 定时时间 * 系统频率 / 分频系数
 *
 * Mode 1 (16-bit): 最大定时 49.152ms @16MHz/12T
 *   1ms 需要计数: 16MHz / 12 * 0.001 = 1333.33 ≈ 1334
 *   重载值 = 65536 - 1334 = 64202
 */
void init_timer0(void)
{
    // 模式1：16位定时器，1ms @16MHz 12T分频
    TL0 = (65536 - 1334);            // 初始值低字节
    TH0 = (65536 - 1334) >> 8;       // 初始值高字节
    TMOD = 0x01;                     // Timer0 模式1 (16位), 系统时钟12分频

    ET0 = 1;                         // 使能 Timer0 中断
    EA  = 1;                         // 使能总中断
    TR0 = 1;                         // 启动 Timer0
}


/*
 * 中断服务函数
 * 中断源：Timer0 中断 (interrupt 1)
 * 每 1ms 重装定时器初值并更新毫秒计数器
 */
void timer0_isr(void) interrupt 1
{
    // 重装初始值 (1ms @16MHz)
    TL0 = (65536 - 1334);
    TH0 = (65536 - 1334) >> 8;

    g_tick_ms++;                     // 毫秒计数递增
}



// ------------------------ 延时函数 ------------------------
// ms 延时
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

    /*
     * 8051 经典 16-bit 定时器读取方式:
     *   先读 TH0, 再读 TL0, 再核对 TH0 是否变化
     *   若 TH0 变了说明 TL0 在读期间溢出进位，需重读
     */
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




// ------------------------ main函数 ------------------------
void main()
{
    // 初始化系统时钟
    SCCON  = 0x00; // HRC
    HRCON |= 0x80;

	  // 配置端口为推挽输出
    P00F = 0X02;
    P01F = 0X02;
    P02F = 0X02;
    P03F = 0X02;
    P04F = 0X02;
    P05F = 0X02;
    P06F = 0X02;
    P07F = 0X02;
    P10F = 0X02;
    P11F = 0X02;
    P12F = 0X02;
    P15F = 0X02;
    P16F = 0X02;
    P17F = 0X02;
    P20F = 0X02;
    P21F = 0X02;

    // 初始化 Timer0
    init_timer0(); 

    while(1)
    {
        // 翻转全部 GPIO（除P13、P14）
        P00 = ~P00;
        P01 = ~P01;
        P02 = ~P02;
        P03 = ~P03;
        P04 = ~P04;
        P05 = ~P05;
        P06 = ~P06;
        P07 = ~P07;
        P10 = ~P10;
        P11 = ~P11;
        P12 = ~P12;
        P15 = ~P15;
        P16 = ~P16;
        P17 = ~P17;
        P20 = ~P20;
        P21 = ~P21;

        delay_ms(500);
    }
}

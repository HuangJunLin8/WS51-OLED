/**
 * oled_disp.c — SSD1306 96x16 OLED 显示驱动实现
 *
 * 架构:
 *   帧缓冲区 (192 字节 XDATA) → OLED_Flush() → 硬件 IIC → SSD1306
 *
 * 帧缓冲区布局 (SSD1306 页寻址模式):
 *   g_oled_fb[page * 96 + col] 表示第 page 页、第 col 列的 8 个垂直像素
 *   每个字节的 bit 0 (LSB) 对应页内顶部像素
 *   像素 (x, y) → 字节索引 = (y/8)*96 + x, 位 = y % 8
 */

#include "oled_disp.h"
#include "oled_i2c.h"

// -------------------- 帧缓冲区 --------------------

// 192 字节帧缓冲区, 放在 XDATA
uint8_t xdata g_oled_fb[OLED_WIDTH * OLED_PAGES];

// 当前字体
static font_t g_font;

// -------------------- 内部函数 --------------------

/**
 * 发送单字节命令到 SSD1306
 */
static void oled_write_cmd(uint8_t cmd)
{
    uint8_t buf;
    buf = cmd;
    OLED_I2C_Send(OLED_I2C_ADDR, 0x00, &buf, 1);
}

/**
 * 发送多字节命令到 SSD1306
 */
static void oled_write_cmds(const uint8_t *cmds, uint8_t len)
{
    OLED_I2C_Send(OLED_I2C_ADDR, 0x00, cmds, len);
}

// -------------------- 公开 API --------------------


// SSD1306 初始化命令序列
void OLED_Init(void)
{
    uint8_t i;

    
    // 显示关闭
    oled_write_cmd(0xAE);


    // 时钟分频 + 振荡频率
    oled_write_cmd(0xD5);
    oled_write_cmd(0x80);


    // 多路复用比 = 15 (16行)
    oled_write_cmd(0xA8);
    oled_write_cmd(0x0F);


    // 显示偏移 = 0
    oled_write_cmd(0xD3);
    oled_write_cmd(0x00);


    // 显示起始行 = 0
    oled_write_cmd(0x40);


    // 电荷泵使能
    oled_write_cmd(0x8D);
    oled_write_cmd(0x14);


    // 页寻址模式
    oled_write_cmd(0x20);
    oled_write_cmd(0x02);


    // 段重映射 (左右翻转)
    oled_write_cmd(0xA1);


    // COM扫描方向 (上下翻转)
    oled_write_cmd(0xC8);


    // COM引脚配置
    oled_write_cmd(0xDA);
    oled_write_cmd(0x02);


    // 对比度
    oled_write_cmd(0x81);
    oled_write_cmd(0xAF);


    // 预充电周期
    oled_write_cmd(0xD9);
    oled_write_cmd(0xF1);


    // VCOMH取消选择电平
    oled_write_cmd(0xDB);
    oled_write_cmd(0x20);


    // 停用滚动
    oled_write_cmd(0x2E);


    // 显示跟随 RAM 内容
    oled_write_cmd(0xA4);


    // 正常显示 (非反转)
    oled_write_cmd(0xA6);


    // 显示开启
    oled_write_cmd(0xAF);


    // -------------------- 第3步: 初始化帧缓冲区和字体 --------------------
    for(i = 0; i < sizeof(g_oled_fb); i++)
    {
        g_oled_fb[i] = 0x00;
    }


    Font_Init(&g_font);
    Font_SetType(&g_font, FONT_Terminus_14);
}


/**
 * 清空帧缓冲区 (全部填充 0)
 */
void OLED_Clear(void)
{
    uint8_t i;
    for (i = 0; i < sizeof(g_oled_fb); i++) {
        g_oled_fb[i] = 0x00;
    }
}

/**
 * 将帧缓冲区刷新到 SSD1306
 *
 * 使用页寻址模式逐页发送:
 *   Page 0: y=0..7   (96 字节)
 *   Page 1: y=8..15  (96 字节)
 */
void OLED_Flush(void)
{
    uint8_t page;
    // col not needed in page mode

    for (page = 0; page < OLED_PAGES; page++) {

        // 设置页地址
        oled_write_cmd((uint8_t)(0xB0 | page));

        // 设置列地址低半字节
        oled_write_cmd(0x00);
        // 设置列地址高半字节
        oled_write_cmd(0x10);

        // 发送本页 96 字节数据
        OLED_I2C_Send(OLED_I2C_ADDR, 0x40,
                      &g_oled_fb[page * OLED_WIDTH],
                      OLED_WIDTH);
    }
}



// ----------------------------------- 基本绘图函数 --------------------------------------

/**
 * 设置/清除单个像素
 *
 * @param x   x 坐标 (0-95)
 * @param y   y 坐标 (0-15)
 * @param on  1=点亮, 0=熄灭
 */
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t on)
{
    uint16_t idx;
    uint8_t  bp;

    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return;

    idx = (uint16_t)((y >> 3) * OLED_WIDTH) + x;
    bp = y & 0x07;

    if (on) {
        g_oled_fb[idx] |= (uint8_t)(1 << bp);
    } else {
        g_oled_fb[idx] &= (uint8_t)(~(1 << bp));
    }
}


/**
 * 绘制水平线段 (供字体解码器回调使用)
 *
 * @param x   起始 x 坐标
 * @param y   起始 y 坐标
 * @param len 像素长度
 * @param on  1=点亮, 0=熄灭
 */
void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t len, uint8_t on)
{
    uint8_t i;
    for (i = 0; i < len; i++) {
        OLED_SetPixel((uint8_t)(x + i), y, on);
    }
}


/**
 * 绘制垂直线段
 *
 * @param x   起始 x 坐标
 * @param y   起始 y 坐标
 * @param len 像素长度
 * @param on  1=点亮, 0=熄灭
 */
void OLED_DrawVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t on)
{
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        OLED_SetPixel(x, (uint8_t)(y + i), on);
    }
}


/**
 * 绘制填充矩形区域
 *
 * @param x   左上角 x 坐标
 * @param y   左上角 y 坐标
 * @param w   矩形宽度
 * @param h   矩形高度
 * @param on  1=点亮, 0=熄灭
 */
void OLED_FillRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t on)
{
    uint8_t row;

    for(row=0;row<h;row++)
    {
        OLED_DrawHLine(x,y+row,w,on);
    }
}


/**
 * 绘制矩形边框 (1 像素宽)
 *
 * @param x   左上角 x 坐标
 * @param y   左上角 y 坐标
 * @param w   矩形宽度
 * @param h   矩形高度
 * @param on  1=点亮, 0=熄灭
 */
void OLED_DrawFrame(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t on)
{
    if(w==0 || h==0)
    {
        return;
    }

    OLED_DrawHLine(x,y,w,on);
    OLED_DrawHLine(x,y+h-1,w,on);

    OLED_DrawVLine(x,y,h,on);
    OLED_DrawVLine(x+w-1,y,h,on);
}

// ----------------------------------- 高级绘图函数 --------------------------------------

/**
 * 绘制任意角度线段 (Bresenham算法)
 *
 * @param x0  起始 x 坐标
 * @param y0  起始 y 坐标
 * @param x1  结束 x 坐标
 * @param y1  结束 y 坐标
 * @param on  1=点亮, 0=熄灭
 */
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t on)
{
    int16_t dx, dy, sx, sy, err, e2;

    dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    dy = -((y1 > y0) ? (y1 - y0) : (y0 - y1));

    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;

    err = dx + dy;

    while (1)
    {
        OLED_SetPixel(x0, y0, on);

        if (x0 == x1 && y0 == y1){
            break;
        }

        e2 = 2 * err;

        if (e2 >= dy){
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx){
            err += dx;
            y0 += sy;
        }
    }
}


// ---------------------- 圆角填充矩形 -------------------------

/**
 * 填充四分之一圆角区域
 *
 * @param cx        圆心 x 坐标
 * @param cy        圆心 y 坐标
 * @param r         圆角半径
 * @param quadrant  圆角方向
 *                  0=左上, 1=右上, 2=左下, 3=右下
 * @param on        1=点亮, 0=熄灭
 */
static void OLED_DrawCornerFill(uint8_t cx,uint8_t cy,uint8_t r,uint8_t quadrant,uint8_t on)
{
    int16_t x=0,y=r,d=3-(r<<1);

    while(x<=y)
    {
        switch(quadrant)
        {
            case 0: // 左上
                OLED_DrawHLine(cx-y,cy-x,y+1,on);
                OLED_DrawHLine(cx-x,cy-y,x+1,on);
                break;

            case 1: // 右上
                OLED_DrawHLine(cx,cy-x,y+1,on);
                OLED_DrawHLine(cx,cy-y,x+1,on);
                break;

            case 2: // 左下
                OLED_DrawHLine(cx-y,cy+x,y+1,on);
                OLED_DrawHLine(cx-x,cy+y,x+1,on);
                break;

            case 3: // 右下
                OLED_DrawHLine(cx,cy+x,y+1,on);
                OLED_DrawHLine(cx,cy+y,x+1,on);
                break;
        }

        if(d<0)
            d+=(x<<2)+6;
        else
        {
            d+=((x-y)<<2)+10;
            y--;
        }

        x++;
    }
}


/**
 * 绘制填充圆角矩形
 *
 * @param x   左上角 x 坐标
 * @param y   左上角 y 坐标
 * @param w   矩形宽度
 * @param h   矩形高度
 * @param r   圆角半径
 * @param on  1=点亮, 0=熄灭
 */
void OLED_DrawRoundedBox(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t r,uint8_t on)
{
    if(r==0 || w<=(r<<1) || h<=(r<<1))
    {
        OLED_FillRect(x,y,w,h,on);
        return;
    }

    // 中央矩形
    OLED_FillRect(x+r,y,w-(r<<1),h,on);

    // 左侧矩形
    OLED_FillRect(x,y+r,r,h-(r<<1),on);

    // 右侧矩形
    OLED_FillRect(x+w-r,y+r,r,h-(r<<1),on);

    // 四个圆角
    OLED_DrawCornerFill(x+r,y+r,r,0,on);
    OLED_DrawCornerFill(x+w-r-1,y+r,r,1,on);
    OLED_DrawCornerFill(x+r,y+h-r-1,r,2,on);
    OLED_DrawCornerFill(x+w-r-1,y+h-r-1,r,3,on);
}


// ---------------------- 圆角矩形框 -------------------------

/**
 * 绘制四分之一圆弧 (Bresenham算法)
 *
 * @param cx        圆心 x 坐标
 * @param cy        圆心 y 坐标
 * @param r         圆弧半径
 * @param quadrant  圆弧方向
 *                  0=左上, 1=右上, 2=左下, 3=右下
 * @param on        1=点亮, 0=熄灭
 */
static void OLED_DrawCornerArc(uint8_t cx,uint8_t cy,uint8_t r,uint8_t quadrant,uint8_t on)
{
    int16_t x=0;
    int16_t y=r;
    int16_t d=3-(r<<1);

    while(x<=y)
    {
        switch(quadrant)
        {
            case 0: // 左上
                OLED_SetPixel(cx-y,cy-x,on);
                OLED_SetPixel(cx-x,cy-y,on);
                break;

            case 1: // 右上
                OLED_SetPixel(cx+y,cy-x,on);
                OLED_SetPixel(cx+x,cy-y,on);
                break;

            case 2: // 左下
                OLED_SetPixel(cx-y,cy+x,on);
                OLED_SetPixel(cx-x,cy+y,on);
                break;

            case 3: // 右下
                OLED_SetPixel(cx+y,cy+x,on);
                OLED_SetPixel(cx+x,cy+y,on);
                break;
        }

        if(d<0)
        {
            d+=(x<<2)+6;
        }
        else
        {
            d+=((x-y)<<2)+10;
            y--;
        }

        x++;
    }
}


/**
 * 绘制圆角矩形框
 *
 * @param x   左上角 x 坐标
 * @param y   左上角 y 坐标
 * @param w   矩形宽度
 * @param h   矩形高度
 * @param r   圆角半径
 * @param on  1=点亮, 0=熄灭
 */
void OLED_DrawRoundedFrame(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t r,uint8_t on)
{
    if(r==0 || w<=(r<<1) || h<=(r<<1))
    {
        OLED_DrawFrame(x,y,w,h,on);
        return;
    }

    // 上边
    OLED_DrawHLine(x+r,y,w-(r<<1),on);

    // 下边
    OLED_DrawHLine(x+r,y+h-1,w-(r<<1),on);

    // 左边
    OLED_DrawVLine(x,y+r,h-(r<<1),on);

    // 右边
    OLED_DrawVLine(x+w-1,y+r,h-(r<<1),on);


    // 四个圆角
    OLED_DrawCornerArc(x+r,y+r,r,0,on);
    OLED_DrawCornerArc(x+w-r-1,y+r,r,1,on);
    OLED_DrawCornerArc(x+r,y+h-r-1,r,2,on);
    OLED_DrawCornerArc(x+w-r-1,y+h-r-1,r,3,on);
}



// ---------------------- 文字显示 -------------------------


/**
 * 设置 OLED 当前字体
 *
 * @param font_buf 字体数据缓冲区
 */
void OLED_SetFont(const uint8_t *font_buf)
{
    Font_SetType(&g_font, font_buf);
}


/**
 * 绘制英文字符串
 *
 * @param x    起始 x 坐标
 * @param y    起始 y 坐标
 * @param str  字符串内容
 */
void OLED_DrawASCII(uint8_t x, uint8_t y, const char *str)
{
    Font_DrawStrASCII(&g_font, x, y, str);
}



void OLED_DrawUTF8(uint8_t x, uint8_t y, const char *str)
{
    Font_DrawStrUTF8(&g_font, x, y, str);
}


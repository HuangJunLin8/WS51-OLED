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

/**
 * 初始化 OLED
 *
 * 初始化顺序:
 *   1. 硬件 IIC 初始化 (P02/P16)
 *   2. SSD1306 命令序列 (基于 u8g2 EastRising 0.69" OLED 初始化)
 *   3. 字体初始化 (FONT_Terminus_14)
 */
void OLED_Init(void)
{
    uint8_t i;

    // -------------------- 第1步: 初始化硬件 IIC --------------------
    OLED_I2C_Init();

    // -------------------- 第2步: SSD1306 初始化命令序列 --------------------
    // 参考: u8x8_d_ssd1306_96x16_er_init_seq (EastRising 0.69" OLED)

    // 显示关闭
    oled_write_cmd(0xAE);

    // 时钟分频 + 振荡频率
    {
        const uint8_t cmd[] = {0xD5, 0x80};
        oled_write_cmds(cmd, 2);
    }

    // 多路复用比 = 15 (16 行)
    {
        const uint8_t cmd[] = {0xA8, 0x0F};
        oled_write_cmds(cmd, 2);
    }

    // 显示偏移 = 0
    {
        const uint8_t cmd[] = {0xD3, 0x00};
        oled_write_cmds(cmd, 2);
    }

    // 显示起始行 = 0
    oled_write_cmd(0x40);

    // 电荷泵使能
    {
        const uint8_t cmd[] = {0x8D, 0x14};
        oled_write_cmds(cmd, 2);
    }

    // 水平寻址模式
    {
        const uint8_t cmd[] = {0x20, 0x00};
        oled_write_cmds(cmd, 2);
    }

    // 段重映射 (左右翻转)
    oled_write_cmd(0xA1);

    // COM 扫描方向 (上下翻转)
    oled_write_cmd(0xC8);

    // COM 引脚配置
    {
        const uint8_t cmd[] = {0xDA, 0x02};
        oled_write_cmds(cmd, 2);
    }

    // 对比度
    {
        const uint8_t cmd[] = {0x81, 0xAF};
        oled_write_cmds(cmd, 2);
    }

    // 预充电周期
    {
        const uint8_t cmd[] = {0xD9, 0xF1};
        oled_write_cmds(cmd, 2);
    }

    // VCOMH 取消选择电平
    {
        const uint8_t cmd[] = {0xDB, 0x20};
        oled_write_cmds(cmd, 2);
    }

    // 停用滚动
    oled_write_cmd(0x2E);

    // 显示跟随 RAM 内容
    oled_write_cmd(0xA4);

    // 正常显示 (非反转)
    oled_write_cmd(0xA6);

    // 显示开启
    oled_write_cmd(0xAF);

    // -------------------- 第3步: 初始化帧缓冲区和字体 --------------------
    for (i = 0; i < sizeof(g_oled_fb); i++) {
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
 * 绘制字符串 (使用默认字体 FONT_Terminus_14)
 */
void OLED_DrawString(uint8_t x, uint8_t y, const char *str)
{
    Font_DrawStr(&g_font, x, y, str);
}

/**
 * 绘制字符串 (使用指定字体)
 */
void OLED_DrawString_F(uint8_t x, uint8_t y, const char *str, font_t *f)
{
    Font_DrawStr(f, x, y, str);
}

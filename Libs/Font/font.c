/**
 * font.c — u8g2 字体解码器实现 (单色, C51 适配)
 *
 * 从 LCD_0-96 参考项目移植，主要改动:
 *   - 移除函数指针回调 → 直接调用 OLED_DrawHLine() (避免 C51 C212 错误)
 *   - 移除 16-bit RGB565 颜色 → 单色 0/1
 *   - 移除 UTF-8 支持，仅 ASCII
 *   - 移除 <string.h> 依赖 (8051 节省空间)
 *   - 所有变量声明在函数开头 (C89 兼容)
 *   - data → buf (避开 C51 data 关键字)
 */

#include "font.h"
#include "oled_disp.h"

/* ==================== 内部辅助函数 ==================== */

/* 读取一个字节 */
static uint8_t font_read_byte(const uint8_t *p)
{
    return *p;
}

/* 读取一个字 (大端序) */
static uint16_t font_read_word(const uint8_t *p)
{
    uint16_t v;
    v  = (uint16_t)(*p) << 8;
    p++;
    v += *p;
    return v;
}

/* 解析字体信息头部 (23 字节) */
static void font_read_info(font_info_t *info, const uint8_t *buf)
{
    info->glyph_cnt          = font_read_byte(buf + 0);
    info->bbx_mode           = font_read_byte(buf + 1);
    info->bits_per_0         = font_read_byte(buf + 2);
    info->bits_per_1         = font_read_byte(buf + 3);
    info->bits_per_char_width  = font_read_byte(buf + 4);
    info->bits_per_char_height = font_read_byte(buf + 5);
    info->bits_per_char_x    = font_read_byte(buf + 6);
    info->bits_per_char_y    = font_read_byte(buf + 7);
    info->bits_per_delta_x   = font_read_byte(buf + 8);
    info->max_char_width     = (int8_t)font_read_byte(buf + 9);
    info->max_char_height    = (int8_t)font_read_byte(buf + 10);
    info->x_offset           = (int8_t)font_read_byte(buf + 11);
    info->y_offset           = (int8_t)font_read_byte(buf + 12);
    info->ascent_A           = (int8_t)font_read_byte(buf + 13);
    info->descent_g          = (int8_t)font_read_byte(buf + 14);
    info->ascent_para        = (int8_t)font_read_byte(buf + 15);
    info->descent_para       = (int8_t)font_read_byte(buf + 16);
    info->start_pos_upper_A  = font_read_word(buf + 17);
    info->start_pos_lower_a  = font_read_word(buf + 19);
    info->start_pos_unicode  = font_read_word(buf + 21);
}

/* 从位流中读取无符号整数 */
static uint8_t font_decode_get_unsigned_bits(font_decode_t *f, uint8_t cnt)
{
    uint8_t val;
    uint8_t bit_pos, bit_pos_plus_cnt;

    bit_pos = f->decode_bit_pos;
    bit_pos_plus_cnt = bit_pos + cnt;

    val = font_read_byte(f->decode_ptr);
    val >>= bit_pos;

    if (bit_pos_plus_cnt >= 8) {
        f->decode_ptr++;
        val |= font_read_byte(f->decode_ptr) << (8 - bit_pos);
        bit_pos_plus_cnt -= 8;
    }
    val &= (1U << cnt) - 1;
    f->decode_bit_pos = bit_pos_plus_cnt;
    return val;
}

/* 从位流中读取有符号整数 */
static int8_t font_decode_get_signed_bits(font_decode_t *f, uint8_t cnt)
{
    int8_t v;
    v = (int8_t)font_decode_get_unsigned_bits(f, cnt);
    v -= (1 << (cnt - 1));
    return v;
}

/* 准备解码一个字形 */
static void font_setup_decode(font_t *f, const uint8_t *glyph_buf)
{
    f->font_decode.decode_ptr      = glyph_buf;
    f->font_decode.decode_bit_pos  = 0;
    f->font_decode.glyph_width     = (int8_t)font_decode_get_unsigned_bits(
        &f->font_decode, f->font_info.bits_per_char_width);
    f->font_decode.glyph_height    = (int8_t)font_decode_get_unsigned_bits(
        &f->font_decode, f->font_info.bits_per_char_height);
}

/**
 * 绘制一段 RLE 编码像素行
 * 直接调用 OLED_DrawHLine() —— 避免 C51 函数指针参数限制
 */
static void font_decode_len(font_t *f, uint8_t len, uint8_t is_foreground)
{
    font_decode_t *decode;
    uint8_t cnt, rem, current;

    decode = &f->font_decode;
    cnt = len;

    for (;;) {
        rem = (uint8_t)decode->glyph_width - (uint8_t)decode->x;
        current = (cnt < rem) ? cnt : rem;

        /* 直接调用显示驱动 (C51 兼容: 不用函数指针) */
        OLED_DrawHLine(
            (uint8_t)(decode->target_x + decode->x),
            (uint8_t)(decode->target_y + decode->y),
            current,
            is_foreground
        );

        if (cnt < rem)
            break;

        cnt -= rem;
        decode->x = 0;
        decode->y++;
    }
    decode->x = (int8_t)((uint8_t)decode->x + cnt);
}

/* 解码一个完整的字形 */
static int8_t font_decode_glyph(font_t *f, const uint8_t *glyph_buf)
{
    font_decode_t *decode;
    int8_t x, y, d;
    int8_t h;
    uint8_t a, b, cont;

    decode = &f->font_decode;
    font_setup_decode(f, glyph_buf);

    x = font_decode_get_signed_bits(decode, f->font_info.bits_per_char_x);
    y = font_decode_get_signed_bits(decode, f->font_info.bits_per_char_y);
    d = font_decode_get_signed_bits(decode, f->font_info.bits_per_delta_x);
    h = decode->glyph_height;

    if (decode->glyph_width > 0) {
        decode->target_x += x;
        decode->target_y -= h + y;
        decode->x = 0;
        decode->y = 0;

        for (;;) {
            a = font_decode_get_unsigned_bits(decode, f->font_info.bits_per_0);
            b = font_decode_get_unsigned_bits(decode, f->font_info.bits_per_1);

            do {
                font_decode_len(f, a, 0);   /* 背景 */
                font_decode_len(f, b, 1);   /* 前景 */
                cont = font_decode_get_unsigned_bits(decode, 1);
            } while (cont != 0);

            if (decode->y >= h)
                break;
        }
    }
    return d;
}

/**
 * 查找指定字符的字形数据
 *
 * @param f        字体对象
 * @param encoding ASCII 字符编码 (0-255)
 * @return 指向字形数据的指针，未找到则返回 NULL
 */
static const uint8_t *font_get_glyph_data(font_t *f, uint16_t encoding)
{
    const uint8_t *font_data;
    uint8_t  len;
    uint8_t  ch;

    font_data = f->font_type + 23;  /* 跳过 23 字节头部 */

    /* 仅支持 ASCII (< 256) */
    if (encoding > 255)
        return (const uint8_t *)0;

    /* 跳转到对应区段 */
    if (encoding >= 'a') {
        font_data += f->font_info.start_pos_lower_a;
    } else if (encoding >= 'A') {
        font_data += f->font_info.start_pos_upper_A;
    }

    /* 线性搜索字形 */
    for (;;) {
        len = font_read_byte(font_data + 1);
        if (len == 0)
            break;

        ch = font_read_byte(font_data);
        if (ch == (uint8_t)encoding) {
            return font_data + 2;       /* 跳过 [编码, 长度] */
        }
        font_data += len;
    }

    return (const uint8_t *)0;
}

/* ==================== 公开 API ==================== */

/**
 * 初始化字体对象
 */
void Font_Init(font_t *f)
{
    uint8_t i;
    uint8_t *p;

    p = (uint8_t *)f;
    for (i = 0; i < sizeof(font_t); i++) {
        p[i] = 0;
    }
}

/**
 * 设置字体类型
 */
void Font_SetType(font_t *f, const uint8_t *font_buf)
{
    if (f->font_type != font_buf) {
        f->font_type = font_buf;
        font_read_info(&f->font_info, font_buf);
    }
}

/**
 * 绘制 ASCII 字符串
 *
 * @param f    字体对象
 * @param x    起始 x 坐标
 * @param y    起始 y 坐标 (基线)
 * @param str  ASCII 字符串 (以 '\0' 结尾)
 * @return 绘制的总宽度 (像素)
 */
uint16_t Font_DrawStr(font_t *f, uint8_t x, uint8_t y, const char *str)
{
    uint16_t sum;
    uint8_t  encoding;
    const char *s;
    const uint8_t *glyph;
    int8_t delta;

    sum = 0;
    s = str;

    /* 按 ascent 调整基线 */
    y = (uint8_t)(y + f->font_info.ascent_A);

    while (*s != '\0') {
        encoding = (uint8_t)(*s);
        s++;

        f->font_decode.target_x = x;
        f->font_decode.target_y = y;

        glyph = font_get_glyph_data(f, encoding);
        if (glyph != (const uint8_t *)0) {
            delta = font_decode_glyph(f, glyph);
            x     = (uint8_t)(x + delta);
            sum   = (uint16_t)(sum + (uint16_t)(uint8_t)delta);
        }
    }
    return sum;
}

/**
 * 获取字符串的像素宽度
 */
uint8_t Font_GetStrWidth(font_t *f, const char *str)
{
    uint8_t width;
    const char *s;
    const uint8_t *glyph;
    int8_t d;

    width = 0;
    s = str;

    while (*s != '\0') {
        glyph = font_get_glyph_data(f, (uint16_t)(uint8_t)(*s));
        s++;

        if (glyph != (const uint8_t *)0) {
            font_setup_decode(f, glyph);
            /* 跳过 x */
            font_decode_get_signed_bits(&f->font_decode,
                                        f->font_info.bits_per_char_x);
            /* 跳过 y */
            font_decode_get_signed_bits(&f->font_decode,
                                        f->font_info.bits_per_char_y);
            /* 读取 delta_x */
            d = font_decode_get_signed_bits(&f->font_decode,
                                            f->font_info.bits_per_delta_x);
            width = (uint8_t)(width + (uint8_t)d);
        }
    }
    return width;
}

/**
 * 获取字体高度 (像素)
 */
uint8_t Font_GetHeight(font_t *f)
{
    return (uint8_t)f->font_info.max_char_height;
}

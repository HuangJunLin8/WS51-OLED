/**
 * font.h — u8g2 字体解码器 (C51 适配, 单色)
 *
 * 从 LCD_0-96 参考项目移植，适配 WS51F6240 (8051) 单色 OLED。
 * 仅支持 u8g2 BDF 编译格式的字体数据 (BBX Mode 0)。
 *
 * C51 适配要点:
 *   - 不使用函数指针回调 (C51 最多 3 个寄存器参数)
 *   - 字体解码器直接调用 OLED_DrawHLine() 绘制像素行
 *
 * 字体数据格式 (23 字节头部):
 *   [0]  glyph_cnt           字形数量
 *   [1]  bbx_mode            0 = BBX mode
 *   [2]  bits_per_0          背景 RLE 编码位数
 *   [3]  bits_per_1          前景 RLE 编码位数
 *   [4]  bits_per_char_width  字形宽度位数
 *   [5]  bits_per_char_height 字形高度位数
 *   [6]  bits_per_char_x      x 偏移位数 (有符号)
 *   [7]  bits_per_char_y      y 偏移位数 (有符号)
 *   [8]  bits_per_delta_x     步进宽度位数 (有符号)
 *   [9]  max_char_width       最大字形宽度 (像素)
 *   [10] max_char_height      最大字形高度 (像素)
 *   [11] x_offset             全局 x 偏移
 *   [12] y_offset             全局 y 偏移
 *   [13] ascent_A             'A' 的上伸部
 *   [14] descent_g            'g' 的下伸部
 *   [15] ascent_para          段落上伸部
 *   [16] descent_para         段落下伸部
 *   [17-18] start_pos_upper_A  大写 'A' 查找表偏移
 *   [19-20] start_pos_lower_a  小写 'a' 查找表偏移
 *   [21-22] start_pos_unicode  Unicode 查找表偏移
 */

#ifndef FONT_H
#define FONT_H

#include "stdint.h"

/* ==================== 字体解码结构体 ==================== */

typedef struct {
    const uint8_t *decode_ptr;
    uint8_t  target_x;
    uint8_t  target_y;
    int8_t   x;
    int8_t   y;
    int8_t   glyph_width;
    int8_t   glyph_height;
    uint8_t  decode_bit_pos;
} font_decode_t;

/* 字体信息 (从字体数据头部解析) */
typedef struct {
    uint8_t  glyph_cnt;
    uint8_t  bbx_mode;
    uint8_t  bits_per_0;
    uint8_t  bits_per_1;
    uint8_t  bits_per_char_width;
    uint8_t  bits_per_char_height;
    uint8_t  bits_per_char_x;
    uint8_t  bits_per_char_y;
    uint8_t  bits_per_delta_x;
    int8_t   max_char_width;
    int8_t   max_char_height;
    int8_t   x_offset;
    int8_t   y_offset;
    int8_t   ascent_A;
    int8_t   descent_g;
    int8_t   ascent_para;
    int8_t   descent_para;
    uint16_t start_pos_upper_A;
    uint16_t start_pos_lower_a;
    uint16_t start_pos_unicode;
} font_info_t;

/* 字体主结构体 */
typedef struct {
    const uint8_t *font_type;       /* 指向字体数据 */
    font_decode_t  font_decode;
    font_info_t    font_info;
} font_t;

/* ==================== 函数声明 ==================== */

void Font_Init(font_t *f);
void Font_SetType(font_t *f, const uint8_t *font_buf);
uint16_t Font_DrawStr(font_t *f, uint8_t x, uint8_t y, const char *str);
uint8_t Font_GetStrWidth(font_t *f, const char *str);
uint8_t Font_GetHeight(font_t *f);

/* ==================== 字体数据声明 ==================== */

extern const uint8_t code FONT_Terminus_14[];

#endif /* FONT_H */

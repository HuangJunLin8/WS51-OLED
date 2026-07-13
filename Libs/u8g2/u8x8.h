/*

  u8x8.h
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
  一个通用的8位图形库

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
  
  
  U8glib有多个层。每个层都通过一个回调函数实现。
  此回调函数处理该层的消息。

  最顶层是显示层。它包含以下消息:
  
    U8X8_MSG_DISPLAY_SETUP_MEMORY			不与显示器通信，只设置内存
    U8X8_MSG_DISPLAY_INIT
    U8X8_MSG_DISPLAY_SET_FLIP_MODE
    U8X8_MSG_DISPLAY_SET_POWER_SAVE
    U8X8_MSG_DISPLAY_SET_CONTRAST
    U8X8_MSG_DISPLAY_DRAW_TILE

  显示硬件可能会决定将这些消息分解到更低级的接口，或者直接实现此功能。
  

  一个层是命令/参数/数据接口。它可以被显示层用于与显示硬件通信。
  此层只处理数据、命令和参数。D/C 线是未知的。
    U8X8_MSG_CAD_INIT
    U8X8_MSG_CAD_SET_I2C_ADR	(已废弃)
    U8X8_MSG_CAD_SET_DEVICE (已废弃)
    U8X8_MSG_CAD_START_TRANSFER
    U8X8_MSG_CAD_SEND_CMD
    U8X8_MSG_CAD_SEND_ARG
    U8X8_MSG_CAD_SEND_DATA
    U8X8_MSG_CAD_END_TRANSFER
    
  字节接口用于向显示硬件发送1字节（8位）数据。
  此层取决于微控制器的硬件，是否应使用特定硬件（I2C或SPI）。
  如果此接口通过软件实现，它可能会使用GPIO级别发送字节。
    U8X8_MSG_BYTE_INIT
    U8X8_MSG_BYTE_SEND 30
    U8X8_MSG_BYTE_SET_DC 31
    U8X8_MSG_BYTE_START_TRANSFER
    U8X8_MSG_BYTE_END_TRANSFER
    U8X8_MSG_BYTE_SET_I2C_ADR (已废弃)
    U8X8_MSG_BYTE_SET_DEVICE (已废弃)

  GPIO和延迟
    U8X8_MSG_GPIO_INIT
    U8X8_MSG_DELAY_MILLI
    U8X8_MSG_DELAY_10MICRO
    U8X8_MSG_DELAY_100NANO
    U8X8_MSG_DELAY_NANO
*/

#ifndef U8X8_H
#define U8X8_H

/*==========================================*/
/* Global Defines */

/* Undefine this to remove u8x8_SetContrast function */
/* 取消定义此宏以移除 u8x8_SetContrast 函数 */
#ifndef U8X8_WITHOUT_SET_CONTRAST
#define U8X8_WITH_SET_CONTRAST
#endif

/* Define this for an additional user pointer inside the u8x8 data struct */
/* 定义此宏以在 u8x8 数据结构中添加一个用户指针 */
//#define U8X8_WITH_USER_PTR


/* Undefine this to remove u8x8_SetFlipMode function */
/* 取消定义此宏以移除 u8x8_SetFlipMode 函数 */
/* 26 May 2016: Obsolete */
/* 2016年5月26日：已废弃 */
//#define U8X8_WITH_SET_FLIP_MODE

/* Select 0 or 1 for the default flip mode. This is not affected by U8X8_WITH_FLIP_MODE */
/* 为默认翻转模式选择 0 或 1。这不受 U8X8_WITH_FLIP_MODE 影响 */
/* Note: Not all display types support a mirror function for the frame buffer */
/* 注意：并非所有显示类型都支持帧缓冲区的镜像功能 */
/* 26 May 2016: Obsolete */
/* 2016年5月26日：已废弃 */
//#define U8X8_DEFAULT_FLIP_MODE 0

/*==========================================*/
/* Includes */


#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>

#if defined(__GNUC__) && defined(__AVR__)
#include <avr/pgmspace.h>
#endif 

/*==========================================*/
/* C++ compatible */

#ifdef __cplusplus
extern "C" {
#endif


/*==========================================*/
/* U8G2 internal defines */

/* the following macro returns the first value for the normal mode */
/* or the second argument for the flip mode */

/* 26 May 2016: Obsolete
#if U8X8_DEFAULT_FLIP_MODE == 0
#define U8X8_IF_DEFAULT_NORMAL_OR_FLIP(normal, flipmode) (normal)
#else
#define U8X8_IF_DEFAULT_NORMAL_OR_FLIP(normal, flipmode) (flipmode)
#endif
*/

#ifdef __GNUC__
#  define U8X8_NOINLINE __attribute__((noinline))
#  define U8X8_SECTION(name) __attribute__ ((section (name)))
#  define U8X8_UNUSED __attribute__((unused))
#else
#  define U8X8_SECTION(name)
#  define U8X8_NOINLINE
#  define U8X8_UNUSED
#endif

#if defined(__GNUC__) && defined(__AVR__)
#  define U8X8_FONT_SECTION(name) U8X8_SECTION(".progmem." name)
#  define u8x8_pgm_read(adr) pgm_read_byte_near(adr)
#  define U8X8_PROGMEM PROGMEM
#endif

#if defined(ESP8266)
uint8_t u8x8_pgm_read_esp(const uint8_t * addr);   /* u8x8_8x8.c */
#  define U8X8_FONT_SECTION(name) __attribute__((section(".text." name)))
#  define u8x8_pgm_read(adr) u8x8_pgm_read_esp(adr)
#  define U8X8_PROGMEM
#endif



#ifndef U8X8_FONT_SECTION
#  define U8X8_FONT_SECTION(name) 
#endif

#ifndef u8x8_pgm_read
#  ifndef CHAR_BIT
#  	define u8x8_pgm_read(adr) (*(const uint8_t *)(adr)) 
#  else
#	if CHAR_BIT > 8 
#  	  define u8x8_pgm_read(adr) ((*(const uint8_t *)(adr)) & 0x0ff)
#     else
#  	  define u8x8_pgm_read(adr) (*(const uint8_t *)(adr)) 
#     endif 
#  endif
#endif

#ifndef U8X8_PROGMEM
#  define U8X8_PROGMEM
#endif

#ifdef ARDUINO
#define U8X8_USE_PINS
#endif

#ifdef __RTTHREAD__
#define U8X8_USE_PINS
#endif

#ifdef __LUATOS__
#define U8X8_USE_PINS
#endif

#if defined(__ARM_LINUX__) || defined(unix) || defined(__unix__) || defined(__unix)
/* https://github.com/olikraus/u8g2/pull/1666 */
#define U8X8_USE_PINS
#define U8X8_WITH_USER_PTR
#endif

/*==========================================*/
/* U8X8 typedefs and data structures */


typedef struct u8x8_struct u8x8_t;
typedef struct u8x8_display_info_struct u8x8_display_info_t;
typedef struct u8x8_tile_struct u8x8_tile_t;

typedef uint8_t (*u8x8_msg_cb)(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
typedef uint16_t (*u8x8_char_cb)(u8x8_t *u8x8, uint8_t b);




//struct u8x8_mcd_struct
//{
//  u8x8_msg_cb cb;		/* current callback function */
//  u8x8_t *u8g2;		/* pointer to the u8g2 parent to minimize the number of args */
//  u8x8_mcd_t *next;
//};

struct u8x8_tile_struct
{
  uint8_t *tile_ptr;	/* 指向一个或多个瓦片的指针... 应该是 "const" */
  uint8_t cnt;		/* 瓦片数量 */
  uint8_t x_pos;	/* 瓦片X位置 */
  uint8_t y_pos;	/* 瓦片Y位置 */
};


struct u8x8_display_info_struct
{
  /* == general == */
  /* == 通用 == */

  uint8_t chip_enable_level;			/* UC1601: 0, 芯片使能电平 */
  uint8_t chip_disable_level;			/* 与 chip_enable_level 相反的电平，芯片禁用电平 */
  
  uint8_t post_chip_enable_wait_ns;		/* UC1601: 5ns, 芯片使能后的等待时间 (纳秒) */
  uint8_t pre_chip_disable_wait_ns;		/* UC1601: 5ns, 芯片禁用前的等待时间 (纳秒) */
  uint8_t reset_pulse_width_ms;		/* UC1601: 0.003ms --> 1ms, 复位脉冲宽度 (毫秒) */ 
  uint8_t post_reset_wait_ms;			/* UC1601: 6ms, 复位后的等待时间 (毫秒) */ 
  
  
  /* == SPI interface == */
  /* == SPI接口 == */
  
  /* 在SDA应用后，等待此时间量以进行SCK数据接收边缘 */
  /* 如果此值小于sck_pulse_width_ns，则使用sck_pulse_width_ns的值 */
  uint8_t sda_setup_time_ns;		/* UC1601: 12ns, SDA建立时间 (纳秒) */
  /* 时钟信号的脉冲宽度，周期时间是此值的两倍 */
  /* 最大频率为 1/(2*sck_pulse_width_ns) */
  /* AVR: 低于70: DIV2, 8 MHz, >= 70 --> 4MHz 时钟 (DIV4) */
  uint8_t sck_pulse_width_ns;		/* UC1701: 50ns, SCK脉冲宽度 (纳秒) */
  
  /* 到目前为止我们有8个字节 (uint8_t)。为SPI.beginTransaction新引入。 */
  uint32_t sck_clock_hz; /* SCK时钟频率 (赫兹) */
  
  /* 之前名称 "sck_takeover_edge" 重命名为 "spi_mode" */
  /* spi_mode 的位 0 等于之前变量 sck_takeover_edge 的值，2016年8月20日：这是错误的，该位实际上是反转的 */ 
  /* SPI有四种时钟模式： */
  /*   0: 时钟高电平有效，数据在下降沿输出，时钟默认值为零，在上升沿接收 */
  /*   1: 时钟高电平有效，数据在上升沿输出，时钟默认值为零，在下降沿接收 */
  /*   2: 时钟低电平有效，数据在上升沿输出 */
  /*   3: 时钟低电平有效，数据在下降沿输出 */
  /* 大多数显示器使用时钟模式 1 */
  uint8_t spi_mode; /* SPI模式 */
  
  /* == I2C == */
  uint8_t i2c_bus_clock_100kHz;		/* UC1601: 1000000000/275 = 37 *100k, I2C总线时钟 (100kHz为单位) */

  
  /* == 8 bit interface == */
  /* == 8位接口 == */
  
  /* 设置所有数据线后的等待时间 */
  uint8_t data_setup_time_ns;		/* UC1601: 30ns, 数据建立时间 (纳秒) */
  /* 写使能脉冲宽度 */
  uint8_t write_pulse_width_ns;		/* UC1601: 40ns, 写脉冲宽度 (纳秒) */
  
  /* == layout == */
  /* == 布局 == */
  uint8_t tile_width; /* 瓦片宽度 */
  uint8_t tile_height; /* 瓦片高度 */

  uint8_t default_x_offset;		/* 显示器的默认X偏移 */
  uint8_t flipmode_x_offset;	/* 翻转模式启用时的X偏移 */
 
 /* 像素宽度不被u8x8过程使用 */
 /* 它将由u8g2过程使用，因为像素尺寸不能总是从tile_width/_height计算出来 */
 /* 以下条件必须为真: */
 /* pixel_width <= tile_width*8 */
 /* pixel_height <= tile_height*8 */
  uint16_t pixel_width; /* 像素宽度 */
  uint16_t pixel_height; /* 像素高度 */
};



/* list of U8x8 pins */
#define U8X8_PIN_D0 0
#define U8X8_PIN_SPI_CLOCK 0
#define U8X8_PIN_D1 1
#define U8X8_PIN_SPI_DATA 1
#define U8X8_PIN_D2 2
#define U8X8_PIN_D3 3
#define U8X8_PIN_D4 4
#define U8X8_PIN_D5 5
#define U8X8_PIN_D6 6
#define U8X8_PIN_D7 7

#define U8X8_PIN_E 8
#define U8X8_PIN_CS 9			/* parallel, SPI */
#define U8X8_PIN_DC 10			/* parallel, SPI */
#define U8X8_PIN_RESET 11		/* parallel, SPI, I2C */

#define U8X8_PIN_I2C_CLOCK 12	/* 1 = Input/high impedance, 0 = drive low */
#define U8X8_PIN_I2C_DATA 13	/* 1 = Input/high impedance, 0 = drive low */

#define U8X8_PIN_CS1 14			/* KS0108 extra chip select */
#define U8X8_PIN_CS2 15			/* KS0108 extra chip select */

#define U8X8_PIN_OUTPUT_CNT 16

#define U8X8_PIN_MENU_SELECT 16
#define U8X8_PIN_MENU_NEXT 17
#define U8X8_PIN_MENU_PREV 18
#define U8X8_PIN_MENU_HOME 19
#define U8X8_PIN_MENU_UP 20
#define U8X8_PIN_MENU_DOWN 21

#define U8X8_PIN_INPUT_CNT 6

#ifdef U8X8_USE_PINS 
#define U8X8_PIN_CNT (U8X8_PIN_OUTPUT_CNT+U8X8_PIN_INPUT_CNT)
#define U8X8_PIN_NONE 255
#endif

struct u8x8_struct
{
  const u8x8_display_info_t *display_info; /* 显示器信息结构体指针 */
  u8x8_char_cb next_cb; /* 从字符串获取下一个字符的函数 */
  u8x8_msg_cb display_cb; /* 显示回调函数 */
  u8x8_msg_cb cad_cb; /* 命令/参数/数据回调函数 */
  u8x8_msg_cb byte_cb; /* 字节回调函数 */
  u8x8_msg_cb gpio_and_delay_cb; /* GPIO和延迟回调函数 */
  uint32_t bus_clock;	/* 可用于字节函数存储总线时钟速度 */
  const uint8_t *font; /* 当前字体 */
  uint16_t encoding;		/* UTF8解码器的编码结果 */
  uint8_t x_offset;	/* 从信息结构体复制，可在翻转模式下修改 */
  uint8_t is_font_inverse_mode; 	/* 0: 正常模式, 1: 字体字形反转 */
  uint8_t i2c_address;	/* 有效的I2C地址。初始为255，但在DISPLAY_INIT期间设置为有用值。 */
					/* i2c_address 是向显示器写入数据的地址 */
					/* 通常，有效地址的最低位必须为零 */
  uint8_t i2c_started;	/* 用于I2C接口 */
  //uint8_t device_address;	/* 已废弃???? - 这是设备地址，U8X8_MSG_CAD_SET_DEVICE 的替代 */
  uint8_t utf8_state;		/* 还需要扫描的字符数 */
  uint8_t gpio_result;	/* GPIO调用的返回值 (目前仅用于菜单键) */ 
  uint8_t debounce_default_pin_state; /* 防抖默认引脚状态 */
  uint8_t debounce_last_pin_state; /* 防抖上次引脚状态 */
  uint8_t debounce_state; /* 防抖状态 */
  uint8_t debounce_result_msg;	/* 防抖后的结果消息或事件 */
#ifdef U8X8_WITH_USER_PTR
  void *user_ptr; /* 用户指针 */
#endif
#ifdef U8X8_USE_PINS 
  uint8_t pins[U8X8_PIN_CNT];	/* 定义引脚列表：主要用于Arduino环境的引脚列表，使用 U8X8_PIN_xxx 访问 */
#endif
};

#ifdef U8X8_WITH_USER_PTR
#define u8x8_GetUserPtr(u8x8) ((u8x8)->user_ptr)
#define u8x8_SetUserPtr(u8x8, p) ((u8x8)->user_ptr = (p))
#endif


#define u8x8_GetCols(u8x8) ((u8x8)->display_info->tile_width)
#define u8x8_GetRows(u8x8) ((u8x8)->display_info->tile_height)
#define u8x8_GetI2CAddress(u8x8) ((u8x8)->i2c_address)
#define u8x8_SetI2CAddress(u8x8, address) ((u8x8)->i2c_address = (address))

#define u8x8_SetGPIOResult(u8x8, val) ((u8x8)->gpio_result = (val))
#define u8x8_GetSPIClockPhase(u8x8) ((u8x8)->display_info->spi_mode & 0x01)  /* 0 means rising edge */
#define u8x8_GetSPIClockPolarity(u8x8) (((u8x8)->display_info->spi_mode & 0x02) >> 1)
#define u8x8_GetSPIClockDefaultLevel(u8x8) (((u8x8)->display_info->spi_mode & 0x02) >> 1)

#define u8x8_GetFontCharWidth(u8x8) u8x8_pgm_read( (u8x8)->font + 2 )
#define u8x8_GetFontCharHeight(u8x8) u8x8_pgm_read( (u8x8)->font + 3 )

#ifdef U8X8_USE_PINS 
#define u8x8_SetPin(u8x8,pin,val) (u8x8)->pins[pin] = (val)
#define u8x8_SetMenuSelectPin(u8x8, val) u8x8_SetPin((u8x8),U8X8_PIN_MENU_SELECT,(val))
#define u8x8_SetMenuNextPin(u8x8, val) u8x8_SetPin((u8x8),U8X8_PIN_MENU_NEXT,(val))
#define u8x8_SetMenuPrevPin(u8x8, val) u8x8_SetPin((u8x8),U8X8_PIN_MENU_PREV,(val))
#define u8x8_SetMenuHomePin(u8x8, val) u8x8_SetPin((u8x8),U8X8_PIN_MENU_HOME,(val))
#define u8x8_SetMenuUpPin(u8x8, val) u8x8_SetPin((u8x8),U8X8_PIN_MENU_UP,(val))
#define u8x8_SetMenuDownPin(u8x8, val) u8x8_SetPin((u8x8),U8X8_PIN_MENU_DOWN,(val))
#endif


/*==========================================*/
/* u8log extension for u8x8 and u8g2 */

typedef struct u8log_struct u8log_t;


/* redraw the specified line. */
typedef void (*u8log_cb)(u8log_t * u8log);

struct u8log_struct
{
  /* configuration */
  void *aux_data;		/* pointer to u8x8 or u8g2 */
  uint8_t width, height;	/* size of the terminal */
  u8log_cb cb;			/* callback redraw function */
  uint8_t *screen_buffer;	/* size must be width*height bytes */
  uint8_t is_redraw_line_for_each_char;
  int8_t line_height_offset;		/* extra offset for the line height (u8g2 only) */
  
  /* internal data */
  //uint8_t last_x, last_y;	/* position of the last printed char */
  uint8_t cursor_x, cursor_y;  /* position of the cursor, might be off screen */
  uint8_t redraw_line;	/* redraw specific line if is_redraw_line is not 0 */
  uint8_t is_redraw_line;
  uint8_t is_redraw_all;
  uint8_t is_redraw_all_required_for_next_nl; /* in nl mode, redraw all instead of current line */
};


/*==========================================*/

/* helper functions */
void u8x8_d_helper_display_setup_memory(u8x8_t *u8x8, const u8x8_display_info_t *display_info);
void u8x8_d_helper_display_init(u8x8_t *u8g2);

/* Display Interface */

/*
  Name: 	U8X8_MSG_DISPLAY_SETUP_MEMORY
  Args:	None
  Tasks:
    1) setup u8g2->display_info
      copy u8g2->display_info->default_x_offset to u8g2->x_offset
      
   usually calls u8x8_d_helper_display_setup_memory()
   名称: U8X8_MSG_DISPLAY_SETUP_MEMORY
   参数: 无
   任务: 1) 设置 u8g2->display_info，将 u8g2->display_info->default_x_offset 复制到 u8g2->x_offset。通常调用 u8x8_d_helper_display_setup_memory()。
*/
#define U8X8_MSG_DISPLAY_SETUP_MEMORY 9

/*
  Name: 	U8X8_MSG_DISPLAY_INIT
  Args:	None
  Tasks:

    2) put interface into default state: 
	  execute u8x8_gpio_Init for port directions
	  execute u8x8_cad_Init for default port levels
    3) set CS status (not clear, may be done in cad/byte interface
    4) execute display reset (gpio interface)
    5) send setup sequence to display, do not activate display, disable "power save" will follow 
   名称: U8X8_MSG_DISPLAY_INIT
   参数: 无
   任务: 2) 将接口设置为默认状态：执行 u8x8_gpio_Init 设置端口方向，执行 u8x8_cad_Init 设置默认端口电平。3) 设置CS状态（不明确，可能在CAD/字节接口中完成）。4) 执行显示器复位（GPIO接口）。5) 向显示器发送设置序列，不激活显示器，后续将禁用“省电模式”。
*/
#define U8X8_MSG_DISPLAY_INIT 10

/*
  Name: 	U8X8_MSG_DISPLAY_SET_POWER_SAVE
  Args:	arg_int: 0: normal mode (RAM is visible on the display), 1: nothing is shown
  Tasks:
    Depending on arg_int, put the display into normal or power save mode.
    Send the corresponding sequence to the display.
    In power save mode, it must be possible to modify the RAM content.
   名称: U8X8_MSG_DISPLAY_SET_POWER_SAVE
   参数: arg_int: 0: 正常模式 (RAM内容在显示器上可见), 1: 不显示任何内容
   任务: 根据 arg_int 的值，将显示器设置为正常模式或省电模式。向显示器发送相应的序列。在省电模式下，必须能够修改RAM内容。
*/
#define U8X8_MSG_DISPLAY_SET_POWER_SAVE 11

/*
  Name: 	U8X8_MSG_DISPLAY_SET_FLIP_MODE
  Args:	arg_int: 0: normal mode, 1: flipped HW screen (180 degree)
  Tasks:
    Reprograms the display controller to rotate the display by 
    180 degree (arg_int = 1) or not (arg_int = 0)
    This may change u8g2->x_offset if the display is smaller than the controller ram
    This message should only be supported if U8X8_WITH_FLIP_MODE is defined.
   名称: U8X8_MSG_DISPLAY_SET_FLIP_MODE
   参数: arg_int: 0: 正常模式, 1: 硬件屏幕翻转 (180度)
   任务: 重新编程显示控制器以旋转显示器180度 (arg_int = 1) 或不旋转 (arg_int = 0)。如果显示器小于控制器RAM，这可能会改变 u8g2->x_offset。仅当定义了 U8X8_WITH_FLIP_MODE 时才支持此消息。
*/
#define U8X8_MSG_DISPLAY_SET_FLIP_MODE 13

/*  arg_int: 0..255 contrast value */
/*  参数: arg_int: 0..255 对比度值 */
#define U8X8_MSG_DISPLAY_SET_CONTRAST 14

/*
  Name: 	U8X8_MSG_DISPLAY_DRAW_TILE
  Args:	
    arg_int: How often to repeat this tile pattern
    arg_ptr: pointer to u8x8_tile_t
        uint8_t *tile_ptr;	pointer to one or more tiles (number is "cnt")
	uint8_t cnt;		number of tiles
	uint8_t x_pos;		first tile x position
	uint8_t y_pos;		first tile y position 
  Tasks:
    One tile has exactly 8 bytes (8x8 pixel monochrome bitmap). 
    The lowest bit of the first byte is the upper left corner
    The highest bit of the first byte is the lower left corner
    The lowest bit of the last byte is the upper right corner
    The highest bit of the last byte is the lower left corner
    "tile_ptr" is the address of a memory area, which contains
    one or more tiles. "cnt" will contain the exact number of
    tiles in the memory areay. The size of the memory area is 8*cnt;
    Multiple tiles in the memory area form a horizontal sequence, this 
    means the first tile is drawn at x_pos/y_pos, the second tile is drawn
    at x_pos+1/y_pos, third at x_pos+2/y_pos.
    "arg_int" tells how often the tile sequence should be repeated:
    For example if "cnt" is two and tile_ptr points to tiles A and B,
    then for arg_int = 3, the following tile sequence will be drawn:
    ABABAB. Totally, cnt*arg_int tiles will be drawn. 
        
   名称: U8X8_MSG_DISPLAY_DRAW_TILE
   参数: arg_int: 重复此瓦片模式的次数
         arg_ptr: 指向 u8x8_tile_t 的指针，其中包含瓦片数据、数量和位置
   任务: 一个瓦片为 8x8 像素的单色位图，占据 8 字节。瓦片数据以水平序列排列，可根据 arg_int 的值重复绘制。
*/
#define U8X8_MSG_DISPLAY_DRAW_TILE 15


/*
  Name: 	U8X8_MSG_DISPLAY_REFRESH
  Args:	
    arg_int: -
    arg_ptr: -
  
  This was introduced for the SSD1606 eInk display.
  The problem is, that all RAM access will not appear on the screen
  unless a special command is executed. With this message, this command
  sequence is executed.
  Use
    void u8x8_RefreshDisplay(u8x8_t *u8x8)
  to send the message to the display handler.
   名称: U8X8_MSG_DISPLAY_REFRESH
   参数: 无
   任务: 用于SSD1606电子墨水显示器，执行特殊命令序列以使RAM内容在屏幕上可见。使用 u8x8_RefreshDisplay(u8x8_t *u8x8) 发送此消息。
*/
#define U8X8_MSG_DISPLAY_REFRESH 16

/*==========================================*/
/* u8x8_setup.c */

uint8_t u8x8_dummy_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 占位回调函数 */

/* 
  设置 u8x8 对象本身。这应该是新 u8x8 对象上调用的第一个函数。
  在此调用后，分配回调函数。可选：设置引脚。
*/

void u8x8_SetupDefaults(u8x8_t *u8x8); /* 设置默认值 (不建议直接使用，请使用 u8x8_Setup()) */

void u8x8_Setup(u8x8_t *u8x8, u8x8_msg_cb display_cb, u8x8_msg_cb cad_cb, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb); /* 初始化 u8x8 对象并设置回调函数 */

/*==========================================*/
/* u8x8_display.c */
uint8_t u8x8_DrawTile(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t cnt, uint8_t *tile_ptr); /* 绘制瓦片 */

/* 
  在调用 u8x8_SetupDefaults 后，设置 u8x8 内存结构并通知回调。
  此函数也从 u8x8_Setup() 调用，因此不要直接调用 u8x8_SetupMemory()，
  而是使用 u8x8_Setup()。
*/
void u8x8_SetupMemory(u8x8_t *u8x8); /* 设置u8x8内存结构 */

/*
  初始化显示器接口，但不初始化显示器本身。
  如果显示器已在运行，这可能很有用。
  
  InitInterface 从 InitDisplay 调用，不要同时调用这两个函数。
*/
void u8x8_InitInterface(u8x8_t *u8x8); /* 初始化显示器接口 */

/*
  在调用 u8x8_SetupMemory()/u8x8_Setup() 后，初始化显示硬件本身。
  这将是 u8x8 第一次与显示器通信。
  它将初始化显示器，但保持显示器在省电模式。
  通常此命令后必须跟随 u8x8_SetPowerSave()。
*/
void u8x8_InitDisplay(u8x8_t *u8x8); /* 初始化显示器硬件 */
/* 从省电模式唤醒显示器 */
void u8x8_SetPowerSave(u8x8_t *u8x8, uint8_t is_enable); /* 设置显示器省电模式 */
void u8x8_SetFlipMode(u8x8_t *u8x8, uint8_t mode); /* 设置显示器翻转模式 */
void u8x8_SetContrast(u8x8_t *u8x8, uint8_t value); /* 设置显示器对比度 */
void u8x8_ClearDisplayWithTile(u8x8_t *u8x8, const uint8_t *buf)  U8X8_NOINLINE; /* 使用瓦片清除显示器 */
void u8x8_ClearDisplay(u8x8_t *u8x8);	// this does not work for u8g2 in some cases /* 清除显示器 (某些情况下不适用于 u8g2) */
void u8x8_FillDisplay(u8x8_t *u8x8); /* 填充显示器 */
void u8x8_RefreshDisplay(u8x8_t *u8x8);	// make RAM content visible on the display (Dec 16: SSD1606 only) /* 刷新显示器 (使RAM内容在显示器上可见，目前仅SSD1606) */
void u8x8_ClearLine(u8x8_t *u8x8, uint8_t line); /* 清除指定行 */



/*==========================================*/
/* Command Arg Data (CAD) Interface */
/* 命令参数数据 (CAD) 接口 */

/*
  U8X8_MSG_CAD_INIT
    no args
    call U8X8_MSG_BYTE_INIT
    setup default values for the I/O lines
   名称: U8X8_MSG_CAD_INIT
   参数: 无
   任务: 调用 U8X8_MSG_BYTE_INIT，设置I/O线的默认值。
*/
#define U8X8_MSG_CAD_INIT 20


#define U8X8_MSG_CAD_SEND_CMD 21 /* 发送命令 */
/*  arg_int: cmd byte */
/*  参数: arg_int: 命令字节 */
#define U8X8_MSG_CAD_SEND_ARG 22 /* 发送参数 */
/*  arg_int: arg byte */
/*  参数: arg_int: 参数字节 */
#define U8X8_MSG_CAD_SEND_DATA 23 /* 发送数据 */
/* arg_int: expected cs level after processing this msg */
/* 参数: arg_int: 处理此消息后预期的CS电平 */
#define U8X8_MSG_CAD_START_TRANSFER 24 /* 开始传输 */
/* arg_int: expected cs level after processing this msg */
/* 参数: arg_int: 处理此消息后预期的CS电平 */
#define U8X8_MSG_CAD_END_TRANSFER 25 /* 结束传输 */
/* arg_int = 0: disable chip, arg_int = 1: enable chip */
/* 参数: arg_int = 0: 禁用芯片, arg_int = 1: 启用芯片 */
//#define U8X8_MSG_CAD_SET_I2C_ADR 26
//#define U8X8_MSG_CAD_SET_DEVICE 27



/* u8g_cad.c */

#define u8x8_cad_Init(u8x8) ((u8x8)->cad_cb((u8x8), U8X8_MSG_CAD_INIT, 0, NULL )) /* 初始化CAD接口 */

uint8_t u8x8_cad_SendCmd(u8x8_t *u8x8, uint8_t cmd) U8X8_NOINLINE; /* 发送单个命令 */
uint8_t u8x8_cad_SendArg(u8x8_t *u8x8, uint8_t arg) U8X8_NOINLINE; /* 发送单个参数 */
uint8_t u8x8_cad_SendMultipleArg(u8x8_t *u8x8, uint8_t cnt, uint8_t arg) U8X8_NOINLINE; /* 发送多个参数 */
uint8_t u8x8_cad_SendData(u8x8_t *u8x8, uint8_t cnt, uint8_t *data) U8X8_NOINLINE; /* 发送数据 */
uint8_t u8x8_cad_StartTransfer(u8x8_t *u8x8) U8X8_NOINLINE; /* 开始传输 */
uint8_t u8x8_cad_EndTransfer(u8x8_t *u8x8) U8X8_NOINLINE; /* 结束传输 */
void u8x8_cad_vsendf(u8x8_t * u8x8, const char *fmt, va_list va); /* 格式化发送CAD数据 */
void u8x8_SendF(u8x8_t * u8x8, const char *fmt, ...); /* 格式化发送CAD数据 (可变参数) */

/*
#define U8X8_C(c0)				(0x04), (c0)
#define U8X8_CA(c0,a0)			(0x05), (c0), (a0)
#define U8X8_CAA(c0,a0,a1)		(0x06), (c0), (a0), (a1)
#define U8X8_DATA()			(0x10)
#define U8X8_D1(d0)			(0x11), (d0)
*/

#define U8X8_C(c0)				(U8X8_MSG_CAD_SEND_CMD), (c0) /* 发送命令 */
#define U8X8_A(a0)				(U8X8_MSG_CAD_SEND_ARG), (a0) /* 发送参数 */
#define U8X8_CA(c0,a0)			(U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0) /* 发送命令和参数 */
#define U8X8_CAA(c0,a0,a1)		(U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1) /* 发送命令和两个参数 */
#define U8X8_CCAA(c0,c1,a0, a1)	(U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_CMD), (c1), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1) /* 发送两个命令和两个参数 */
#define U8X8_CAAA(c0,a0,a1, a2)	(U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1), (U8X8_MSG_CAD_SEND_ARG), (a2) /* 发送命令和三个参数 */
#define U8X8_CAAAA(c0,a0,a1,a2,a3)		(U8X8_MSG_CAD_SEND_CMD), (c0), (U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1), (U8X8_MSG_CAD_SEND_ARG), (a2), (U8X8_MSG_CAD_SEND_ARG), (a3) /* 发送命令和四个参数 */
#define U8X8_AAC(a0,a1,c0)		(U8X8_MSG_CAD_SEND_ARG), (a0), (U8X8_MSG_CAD_SEND_ARG), (a1), (U8X8_MSG_CAD_SEND_CMD), (c0) /* 发送两个参数和一个命令 */
#define U8X8_D1(d0)			(U8X8_MSG_CAD_SEND_DATA), (d0) /* 发送一个数据字节 */

#define U8X8_A4(a0,a1,a2,a3)		U8X8_A(a0), U8X8_A(a1), U8X8_A(a2), U8X8_A(a3) /* 发送四个参数 */
#define U8X8_A8(a0,a1,a2,a3,a4,a5,a6,a7)	U8X8_A4((a0), (a1), (a2), (a3)), U8X8_A4((a4), (a5), (a6), (a7)) /* 发送八个参数 */


#define U8X8_START_TRANSFER()	(U8X8_MSG_CAD_START_TRANSFER) /* 开始传输 */
#define U8X8_END_TRANSFER()	(U8X8_MSG_CAD_END_TRANSFER) /* 结束传输 */
#define U8X8_DLY(m)			(0xfe),(m)		/* 延迟 (毫秒) */
#define U8X8_END()			(0xff) /* 序列结束标记 */

void u8x8_cad_SendSequence(u8x8_t *u8x8, uint8_t const *data); /* 发送命令/参数/数据序列 */
uint8_t u8x8_cad_empty(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 空的CAD回调函数 */
uint8_t u8x8_cad_110(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* CAD回调函数 (110模式) */
uint8_t u8x8_gu800_cad_110(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* GU800的CAD回调函数 (110模式) */
uint8_t u8x8_cad_001(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* CAD回调函数 (001模式) */
uint8_t u8x8_cad_011(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* CAD回调函数 (011模式) */
uint8_t u8x8_cad_100(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* CAD回调函数 (100模式) */
uint8_t u8x8_cad_st7920_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* ST7920的SPI CAD回调函数 */
uint8_t u8x8_cad_ssd13xx_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* SSD13xx的I2C CAD回调函数 */
uint8_t u8x8_cad_ssd13xx_fast_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* SSD13xx的快速I2C CAD回调函数 */
uint8_t u8x8_cad_st75256_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* ST75256的I2C CAD回调函数 */
uint8_t u8x8_cad_ld7032_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* LD7032的I2C CAD回调函数 */
uint8_t u8x8_cad_uc16xx_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);  /* UC16xx的I2C CAD回调函数 (CAD=001) */
uint8_t u8x8_cad_uc1638_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);  /* UC1638的I2C CAD回调函数 (CAD=011) */


/*==========================================*/
/* Byte Interface */
/* 字节接口 */

#define U8X8_MSG_BYTE_INIT U8X8_MSG_CAD_INIT /* 字节初始化消息 */
#define U8X8_MSG_BYTE_SET_DC 32 /* 设置数据/命令线 */

#define U8X8_MSG_BYTE_SEND U8X8_MSG_CAD_SEND_DATA /* 发送字节数据 */

#define U8X8_MSG_BYTE_START_TRANSFER U8X8_MSG_CAD_START_TRANSFER /* 开始字节传输 */
#define U8X8_MSG_BYTE_END_TRANSFER U8X8_MSG_CAD_END_TRANSFER /* 结束字节传输 */

//#define U8X8_MSG_BYTE_SET_I2C_ADR U8X8_MSG_CAD_SET_I2C_ADR
//#define U8X8_MSG_BYTE_SET_DEVICE U8X8_MSG_CAD_SET_DEVICE


uint8_t u8x8_byte_SetDC(u8x8_t *u8x8, uint8_t dc) U8X8_NOINLINE; /* 设置DC线电平 */
uint8_t u8x8_byte_SendByte(u8x8_t *u8x8, uint8_t byte) U8X8_NOINLINE; /* 发送单个字节 */
uint8_t u8x8_byte_SendBytes(u8x8_t *u8x8, uint8_t cnt, uint8_t *data) U8X8_NOINLINE; /* 发送多个字节 */
uint8_t u8x8_byte_StartTransfer(u8x8_t *u8x8); /* 开始字节传输 */
uint8_t u8x8_byte_EndTransfer(u8x8_t *u8x8); /* 结束字节传输 */

uint8_t u8x8_byte_empty(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 空的字节回调函数 */
uint8_t u8x8_byte_4wire_sw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 4线软件SPI字节回调函数 */
uint8_t u8x8_byte_8bit_6800mode(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 8位6800模式字节回调函数 */
uint8_t u8x8_byte_8bit_8080mode(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 8位8080模式字节回调函数 */
uint8_t u8x8_byte_3wire_sw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 3线软件SPI字节回调函数 */
/* uint8_t u8x8_byte_st7920_sw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); */ /* ST7920软件SPI字节回调函数 */
void u8x8_byte_set_ks0108_cs(u8x8_t *u8x8, uint8_t arg) U8X8_NOINLINE; /* 设置KS0108的CS线 */
uint8_t u8x8_byte_ks0108(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* KS0108字节回调函数 */
uint8_t u8x8_byte_ssd13xx_sw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);  /* SSD13xx的软件I2C字节回调函数 (已废弃!) */
uint8_t u8x8_byte_sw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 软件I2C字节回调函数 */
uint8_t u8x8_byte_sed1520(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* SED1520字节回调函数 */


/*==========================================*/
/* GPIO Interface */
/* GPIO接口 */


/*
  U8X8_MSG_GPIO_AND_DELAY_INIT
  no args
  setup port directions, do not set IO levels, this is done with BYTE/CAD_INIT
   名称: U8X8_MSG_GPIO_AND_DELAY_INIT
   参数: 无
   任务: 设置端口方向，不设置I/O电平，这在BYTE/CAD_INIT中完成。
*/
#define U8X8_MSG_GPIO_AND_DELAY_INIT 40

/* arg_int: milliseconds */
/* 参数: arg_int: 毫秒 */
#define U8X8_MSG_DELAY_MILLI		41 /* 延迟毫秒 */

/* 10MICRO and 100NANO are not used at the moment */
/* 10微秒和100纳秒目前未使用 */
#define U8X8_MSG_DELAY_10MICRO		42 /* 延迟10微秒 */
#define U8X8_MSG_DELAY_100NANO		43 /* 延迟100纳秒 */


#define U8X8_MSG_DELAY_NANO		44 /* 延迟纳秒 */
/* delay of one i2c unit, should be 5us for 100K, and 1.25us for 400K */
/* 一个I2C单位的延迟，100K时应为5us，400K时为1.25us */
#define U8X8_MSG_DELAY_I2C		45 /* I2C延迟 */

#define U8X8_MSG_GPIO(x) (64+(x))
#ifdef U8X8_USE_PINS 
#define u8x8_GetPinIndex(u8x8, msg) ((msg)&0x3f) /* 获取引脚索引 */
#define u8x8_GetPinValue(u8x8, msg) ((u8x8)->pins[(msg)&0x3f]) /* 获取引脚值 */
#endif

#define U8X8_MSG_GPIO_D0			U8X8_MSG_GPIO(U8X8_PIN_D0) /* GPIO D0 */
#define U8X8_MSG_GPIO_SPI_CLOCK	U8X8_MSG_GPIO(U8X8_PIN_SPI_CLOCK) /* GPIO SPI时钟 */
#define U8X8_MSG_GPIO_D1			U8X8_MSG_GPIO(U8X8_PIN_D1) /* GPIO D1 */
#define U8X8_MSG_GPIO_SPI_DATA		U8X8_MSG_GPIO(U8X8_PIN_SPI_DATA) /* GPIO SPI数据 */
#define U8X8_MSG_GPIO_D2			U8X8_MSG_GPIO(U8X8_PIN_D2) /* GPIO D2 */
#define U8X8_MSG_GPIO_D3			U8X8_MSG_GPIO(U8X8_PIN_D3) /* GPIO D3 */
#define U8X8_MSG_GPIO_D4			U8X8_MSG_GPIO(U8X8_PIN_D4) /* GPIO D4 */
#define U8X8_MSG_GPIO_D5			U8X8_MSG_GPIO(U8X8_PIN_D5) /* GPIO D5 */
#define U8X8_MSG_GPIO_D6			U8X8_MSG_GPIO(U8X8_PIN_D6) /* GPIO D6 */
#define U8X8_MSG_GPIO_D7			U8X8_MSG_GPIO(U8X8_PIN_D7) /* GPIO D7 */
#define U8X8_MSG_GPIO_E 			U8X8_MSG_GPIO(U8X8_PIN_E)			// used as E1 for the SED1520 /* GPIO E (用于SED1520的E1) */
#define U8X8_MSG_GPIO_CS			U8X8_MSG_GPIO(U8X8_PIN_CS)		// used as E2 for the SED1520 /* GPIO CS (用于SED1520的E2) */
#define U8X8_MSG_GPIO_DC			U8X8_MSG_GPIO(U8X8_PIN_DC) /* GPIO DC */
#define U8X8_MSG_GPIO_RESET 		U8X8_MSG_GPIO(U8X8_PIN_RESET) /* GPIO 复位 */
#define U8X8_MSG_GPIO_I2C_CLOCK	U8X8_MSG_GPIO(U8X8_PIN_I2C_CLOCK) /* GPIO I2C时钟 */
#define U8X8_MSG_GPIO_I2C_DATA		U8X8_MSG_GPIO(U8X8_PIN_I2C_DATA) /* GPIO I2C数据 */


#define U8X8_MSG_GPIO_CS1			U8X8_MSG_GPIO(U8X8_PIN_CS1)	/* KS0108 额外的片选线1 */
#define U8X8_MSG_GPIO_CS2			U8X8_MSG_GPIO(U8X8_PIN_CS2)	/* KS0108 额外的片选线2 */


/* these message expect the return value in u8x8->gpio_result */
/* 这些消息期望返回值在 u8x8->gpio_result 中 */
#define U8X8_MSG_GPIO_MENU_SELECT	U8X8_MSG_GPIO(U8X8_PIN_MENU_SELECT) /* GPIO 菜单选择 */
#define U8X8_MSG_GPIO_MENU_NEXT	U8X8_MSG_GPIO(U8X8_PIN_MENU_NEXT) /* GPIO 菜单下一项 */
#define U8X8_MSG_GPIO_MENU_PREV	U8X8_MSG_GPIO(U8X8_PIN_MENU_PREV) /* GPIO 菜单上一项 */
#define U8X8_MSG_GPIO_MENU_HOME	U8X8_MSG_GPIO(U8X8_PIN_MENU_HOME) /* GPIO 菜单主页 */
#define U8X8_MSG_GPIO_MENU_UP		U8X8_MSG_GPIO(U8X8_PIN_MENU_UP) /* GPIO 菜单上移 */
#define U8X8_MSG_GPIO_MENU_DOWN	U8X8_MSG_GPIO(U8X8_PIN_MENU_DOWN) /* GPIO 菜单下移 */


#define u8x8_gpio_Init(u8x8) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_AND_DELAY_INIT, 0, NULL )) /* 初始化GPIO */


/*
#define u8x8_gpio_SetDC(u8x8, v) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_DC, (v), NULL ))
#define u8x8_gpio_SetCS(u8x8, v) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_CS, (v), NULL ))
#define u8x8_gpio_SetReset(u8x8, v) ((u8x8)->gpio_and_delay_cb((u8x8), U8X8_MSG_GPIO_RESET, (v), NULL ))
*/

#define u8x8_gpio_SetDC(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_DC, (v)) /* 设置DC引脚 */
#define u8x8_gpio_SetCS(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_CS, (v)) /* 设置CS引脚 */
#define u8x8_gpio_SetReset(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_RESET, (v)) /* 设置复位引脚 */
#define u8x8_gpio_SetSPIClock(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_SPI_CLOCK, (v)) /* 设置SPI时钟引脚 */
#define u8x8_gpio_SetSPIData(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_SPI_DATA, (v)) /* 设置SPI数据引脚 */
#define u8x8_gpio_SetI2CClock(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_I2C_CLOCK, (v)) /* 设置I2C时钟引脚 */
#define u8x8_gpio_SetI2CData(u8x8, v) u8x8_gpio_call(u8x8, U8X8_MSG_GPIO_I2C_DATA, (v)) /* 设置I2C数据引脚 */

void u8x8_gpio_call(u8x8_t *u8x8, uint8_t msg, uint8_t arg) U8X8_NOINLINE; /* 调用GPIO回调函数 */

#define u8x8_gpio_Delay(u8x8, msg, dly) u8x8_gpio_call((u8x8), (msg), (dly)) /* GPIO延迟 */
//void u8x8_gpio_Delay(u8x8_t *u8x8, uint8_t msg, uint8_t dly) U8X8_NOINLINE;


/*==========================================*/
/* u8x8_debounce.c */
/* return U8X8_MSG_GPIO_MENU_xxxxx messages */
uint8_t u8x8_GetMenuEvent(u8x8_t *u8x8);

/*==========================================*/
/* u8x8_d_stdio.c */
void u8x8_SetupStdio(u8x8_t *u8x8);

/*==========================================*/
/* u8x8_d_sdl_128x64.c */
void u8x8_Setup_SDL_128x64(u8x8_t *u8x8);
void u8x8_Setup_SDL_240x160(u8x8_t *u8x8);
int u8g_sdl_get_key(void);

/*==========================================*/
/* u8x8_d_tga.c */
void u8x8_Setup_TGA_DESC(u8x8_t *u8x8);
void u8x8_Setup_TGA_LCD(u8x8_t *u8x8);
void tga_save(const char *name);

/*==========================================*/
/* u8x8_d_bitmap.c */
uint8_t u8x8_GetBitmapPixel(u8x8_t *u8x8, uint16_t x, uint16_t y);
void u8x8_SaveBitmapTGA(u8x8_t *u8x8, const char *filename);
void u8x8_SetupBitmap(u8x8_t *u8x8, uint16_t pixel_width, uint16_t pixel_height);
uint8_t u8x8_ConnectBitmapToU8x8(u8x8_t *u8x8);

/*==========================================*/
/* u8x8_d_utf8.c */
void u8x8_Setup_Utf8(u8x8_t *u8x8);	/* stdout UTF-8 display */
void utf8_show(void);		/* show content of UTF-8 frame buffer */


/*==========================================*/

/* u8x8_setup.c */
uint8_t u8x8_d_null_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/* u8x8_d_XXX.c */
uint8_t u8x8_d_uc1701_ea_dogs102(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1701_mini12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1305_128x32_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1305_128x32_adafruit(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
/*uint8_t u8x8_d_ssd1305_128x32_waveshare(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);*/
uint8_t u8x8_d_ssd1305_128x64_adafruit(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1305_128x64_raystar(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_128x64_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_128x64_vcomh0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_128x64_alt0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1309_128x64_noname0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1309_128x64_noname2(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1312_128x64_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_2040x16(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_128x32_univision(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_128x32_winstar(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_102x64_ea_oleds102(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);  /* u8x8_ssd1309.c */
uint8_t u8x8_d_ssd1306_64x48_er(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_48x64_winstar(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_64x32_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_64x32_1f(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_96x16_er(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_96x40(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_96x39(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1306_72x40_er(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1106_128x64_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1106_128x64_vcomh0(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1106_128x64_winstar(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1106_128x32_visionox(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); // located in ssd1306_128x32
uint8_t u8x8_d_sh1106_72x40_wise(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1106_64x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_64x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_seeed_96x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_128x80(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_pimoroni_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_seeed_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_hjr_oel1m0201_96x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1107_tk078f288_80x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1108_128x160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1108_160x160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sh1122_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7920_160x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7920_192x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7920_256x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7920_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ls013b7dh03_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ls027b7dh01_400x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ls027b7dh01_m0_400x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ls013b7dh05_144x168(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7511_avd_320x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7528_nhd_c160100(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7528_erc16064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_ea_dogm128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_lm6063(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_64128n(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_ea_dogm132(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_zolen_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_nhd_c12832(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_nhd_c12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_jlx12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_lm6059(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_ks0713(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_lx12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_erc12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7565_erc12864_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);  /* issue #790 */
uint8_t u8x8_d_st7567_pi_132x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_jlx12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_122x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_enh_dg128064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_enh_dg128064i(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_64x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_hem6432(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_os12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7567_erc13232(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7571_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7571_128x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7586s_s028hn118a(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7586s_jlx384160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7586s_erc240160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7586s_ymc240160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st7588_jlx12864(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75160_jm16096(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx256128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_wo256x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx256160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx256160m(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx256160_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx240160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx25664(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx172104(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx19296(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75256_jlx16080(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_st75320_jlx320240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);	/* https://github.com/olikraus/u8g2/issues/921 */
uint8_t u8x8_d_nt7534_tg12864r(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* u8x8_d_st7565.c */
uint8_t u8x8_d_ld7032_60x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ld7032_60x32_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t6963_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t6963_240x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t6963_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t6963_128x64_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t6963_160x80(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t6963_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1316_128x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1316_96x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1317_96x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1318_128x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1318_128x96_xcp(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1320_160x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1320_160x132(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1320_160x80(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1322_nhd_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1322_nhd_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1362_256x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1362_206x36(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_a2printer_384x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sed1330_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ra8835_nhd_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ra8835_320x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1325_nhd_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd0323_os128064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_ws_96x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_seeed_96x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_ea_w128128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_midas_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_zjy_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_ws_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1327_visionox_128x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1326_er_256x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1329_128x96_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1329_96x96_noname(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1601_128x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1601_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1604_jlx19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1608_erc24064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1608_dem240064(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1608_erc240120(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1608_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1609_slg19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1610_ea_dogxl160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1611_ea_dogm240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1611_ea_dogxl240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1611_ew50850(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);  /* 240x160 */
uint8_t u8x8_d_uc1611_cg160160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 160x160 */
uint8_t u8x8_d_uc1617_jlx128128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1611_ids4073(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* 256x128 */
uint8_t u8x8_d_uc1638_160x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1638_192x96(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_uc1638_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ks0108_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ks0108_erm19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_t7932_150x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* t7932 and hd44102 are compatible */
uint8_t u8x8_d_hd44102_100x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* t7932 and hd44102 are compatible */
uint8_t u8x8_d_sbn1661_122x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_sed1520_122x32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_pcd8544_84x48(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_pcf8812_96x65(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_hx1230_96x68(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1606_172x72(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1607_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1607_v2_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1607_gd_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ssd1607_ws_200x200(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr); /* issue 637 */
uint8_t u8x8_d_il3820_296x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_il3820_v2_296x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_lc7981_160x80(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_lc7981_160x160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_lc7981_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_lc7981_240x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_lc7981_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ist3020_erc19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ist3088_320x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_ist7920_128x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_max7219_64x8(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_max7219_32x8(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_max7219_16x16(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_max7219_8x8(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_s1d15e06_160100(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_s1d15300_lm6023(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_s1d15721_240x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_gu800_128x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_gu800_160x16(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_gp1287ai_256x50(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_d_gp1247ai_253x63(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);



/*==========================================*/
/* u8x8_8x8.c */

uint16_t u8x8_upscale_byte(uint8_t x) U8X8_NOINLINE; /* 放大字节 */


void u8x8_utf8_init(u8x8_t *u8x8); /* 初始化UTF8解码器 */
uint16_t u8x8_ascii_next(u8x8_t *u8x8, uint8_t b); /* 获取下一个ASCII字符 */
uint16_t u8x8_utf8_next(u8x8_t *u8x8, uint8_t b); /* 获取下一个UTF8字符 */
// the following two functions are replaced by the init/next functions 
// 以下两个函数被init/next函数替换
//uint16_t u8x8_get_encoding_from_utf8_string(const char **str);
//uint16_t u8x8_get_char_from_string(const char **str);

void u8x8_SetFont(u8x8_t *u8x8, const uint8_t *font_8x8); /* 设置当前字体 (8x8) */
void u8x8_DrawGlyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding); /* 绘制字形 */
void u8x8_Draw2x2Glyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding); /* 绘制2x2倍大的字形 */
void u8x8_Draw1x2Glyph(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t encoding); /* 绘制1x2倍大的字形 */
uint8_t u8x8_DrawString(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s); /* 绘制字符串 */
uint8_t u8x8_DrawUTF8(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s);	/* return number of glyps */ /* 绘制UTF8字符串 (返回字形数量) */
uint8_t u8x8_Draw2x2String(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s); /* 绘制2x2倍大的字符串 */
uint8_t u8x8_Draw2x2UTF8(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s); /* 绘制2x2倍大的UTF8字符串 */
uint8_t u8x8_Draw1x2String(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s); /* 绘制1x2倍大的字符串 */
uint8_t u8x8_Draw1x2UTF8(u8x8_t *u8x8, uint8_t x, uint8_t y, const char *s); /* 绘制1x2倍大的UTF8字符串 */
uint8_t u8x8_GetUTF8Len(u8x8_t *u8x8, const char *s); /* 获取UTF8字符串长度 */
#define u8x8_SetInverseFont(u8x8, b) (u8x8)->is_font_inverse_mode = (b) /* 设置反色字体 */

/*==========================================*/
/* itoa procedures */
/* 整数转字符串过程 */
const char *u8x8_u8toa(uint8_t v, uint8_t d); /* 8位无符号整数转字符串 */
const char *u8x8_u16toa(uint16_t v, uint8_t d); /* 16位无符号整数转字符串 */
const char *u8x8_utoa(uint16_t v); /* 无符号整数转字符串 */


/*==========================================*/
/* u8x8_string.c */

uint8_t u8x8_GetStringLineCnt(const char *str);  /* return 0 for str==NULL */ /* 获取字符串行数 (如果str==NULL则返回0) */
const char *u8x8_GetStringLineStart(uint8_t line_idx, const char *str ); /* 获取指定行的起始位置 */
void u8x8_CopyStringLine(char *dest, uint8_t line_idx, const char *str); /* 复制指定行到目标地址 */
/* draw one line, consider \t for center */
/* 绘制一行，考虑\t用于居中 */
uint8_t u8x8_DrawUTF8Line(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t w, const char *s); /* 绘制UTF8行 */
/* draw multiple lines, handle \t */
/* 绘制多行，处理\t */
uint8_t u8x8_DrawUTF8Lines(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t w, const char *s); /* 绘制多行UTF8文本 */

/*==========================================*/

/* u8x8_selection_list.c */
struct _u8sl_struct
{
  uint8_t visible;		/* number of visible elements in the menu */
  uint8_t total;			/* total number of elements in the menu */
  uint8_t first_pos;		/* position of the first visible line */
  uint8_t current_pos;	/* current cursor position, starts at 0 */  
  
  uint8_t x;		/* u8x8 only, not used in u8g2 */
  uint8_t y;		/* u8x8 only, not used in u8g2 */
};
typedef struct _u8sl_struct u8sl_t;

typedef void (*u8x8_sl_cb)(u8x8_t *u8x8, u8sl_t *u8sl, uint8_t idx, const void *aux);

void u8sl_Next(u8sl_t *u8sl);
void u8sl_Prev(u8sl_t *u8sl);

uint8_t u8x8_UserInterfaceSelectionList(u8x8_t *u8x8, const char *title, uint8_t start_pos, const char *sl);

/*==========================================*/

/* u8x8_message.c  */
uint8_t u8x8_UserInterfaceMessage(u8x8_t *u8x8, const char *title1, const char *title2, const char *title3, const char *buttons);

/*==========================================*/
/* u8x8_capture.c */

/* vertical_top memory architecture */
uint8_t u8x8_capture_get_pixel_1(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width);

/* horizontal right memory architecture */
/* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */ 
uint8_t u8x8_capture_get_pixel_2(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width);



void u8x8_capture_write_pbm_pre(uint8_t tile_width, uint8_t tile_height, void (*out)(const char *s));
void u8x8_capture_write_pbm_buffer(uint8_t *buffer, uint8_t tile_width, uint8_t tile_height, uint8_t (*get_pixel)(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width), void (*out)(const char *s));

void u8x8_capture_write_xbm_pre(uint8_t tile_width, uint8_t tile_height, void (*out)(const char *s));
void u8x8_capture_write_xbm_buffer(uint8_t *buffer, uint8_t tile_width, uint8_t tile_height, uint8_t (*get_pixel)(uint16_t x, uint16_t y, uint8_t *dest_ptr, uint8_t tile_width), void (*out)(const char *s));



/*==========================================*/

/* u8x8_input_value.c  */

uint8_t u8x8_UserInterfaceInputValue(u8x8_t *u8x8, const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post);

/*==========================================*/
/* u8log.c */
void u8log_Init(u8log_t *u8log, uint8_t width, uint8_t height, uint8_t *buf);
void u8log_SetCallback(u8log_t *u8log, u8log_cb cb, void *aux_data);
void u8log_SetRedrawMode(u8log_t *u8log, uint8_t is_redraw_line_for_each_char);
void u8log_SetLineHeightOffset(u8log_t *u8log, int8_t line_height_offset);
void u8log_WriteString(u8log_t *u8log, const char *s) U8X8_NOINLINE;
void u8log_WriteChar(u8log_t *u8log, uint8_t c) U8X8_NOINLINE;
void u8log_WriteHex8(u8log_t *u8log, uint8_t b) U8X8_NOINLINE;
void u8log_WriteHex16(u8log_t *u8log, uint16_t v);
void u8log_WriteHex32(u8log_t *u8log, uint32_t v);
void u8log_WriteDec8(u8log_t *u8log, uint8_t v, uint8_t d);
void u8log_WriteDec16(u8log_t *u8log, uint16_t v, uint8_t d);

/*==========================================*/
/* u8log_u8x8.c */
void u8x8_DrawLog(u8x8_t *u8x8, uint8_t x, uint8_t y, u8log_t *u8log);
void u8log_u8x8_cb(u8log_t * u8log);


/*==========================================*/
/* start font list */
extern const uint8_t u8x8_font_amstrad_cpc_extended_f[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_f");
extern const uint8_t u8x8_font_amstrad_cpc_extended_r[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_r");
extern const uint8_t u8x8_font_amstrad_cpc_extended_n[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_n");
extern const uint8_t u8x8_font_amstrad_cpc_extended_u[] U8X8_FONT_SECTION("u8x8_font_amstrad_cpc_extended_u");
extern const uint8_t u8x8_font_5x7_f[] U8X8_FONT_SECTION("u8x8_font_5x7_f");
extern const uint8_t u8x8_font_5x7_r[] U8X8_FONT_SECTION("u8x8_font_5x7_r");
extern const uint8_t u8x8_font_5x7_n[] U8X8_FONT_SECTION("u8x8_font_5x7_n");
extern const uint8_t u8x8_font_5x8_f[] U8X8_FONT_SECTION("u8x8_font_5x8_f");
extern const uint8_t u8x8_font_5x8_r[] U8X8_FONT_SECTION("u8x8_font_5x8_r");
extern const uint8_t u8x8_font_5x8_n[] U8X8_FONT_SECTION("u8x8_font_5x8_n");
extern const uint8_t u8x8_font_8x13_1x2_f[] U8X8_FONT_SECTION("u8x8_font_8x13_1x2_f");
extern const uint8_t u8x8_font_8x13_1x2_r[] U8X8_FONT_SECTION("u8x8_font_8x13_1x2_r");
extern const uint8_t u8x8_font_8x13_1x2_n[] U8X8_FONT_SECTION("u8x8_font_8x13_1x2_n");
extern const uint8_t u8x8_font_8x13B_1x2_f[] U8X8_FONT_SECTION("u8x8_font_8x13B_1x2_f");
extern const uint8_t u8x8_font_8x13B_1x2_r[] U8X8_FONT_SECTION("u8x8_font_8x13B_1x2_r");
extern const uint8_t u8x8_font_8x13B_1x2_n[] U8X8_FONT_SECTION("u8x8_font_8x13B_1x2_n");
extern const uint8_t u8x8_font_7x14_1x2_f[] U8X8_FONT_SECTION("u8x8_font_7x14_1x2_f");
extern const uint8_t u8x8_font_7x14_1x2_r[] U8X8_FONT_SECTION("u8x8_font_7x14_1x2_r");
extern const uint8_t u8x8_font_7x14_1x2_n[] U8X8_FONT_SECTION("u8x8_font_7x14_1x2_n");
extern const uint8_t u8x8_font_7x14B_1x2_f[] U8X8_FONT_SECTION("u8x8_font_7x14B_1x2_f");
extern const uint8_t u8x8_font_7x14B_1x2_r[] U8X8_FONT_SECTION("u8x8_font_7x14B_1x2_r");
extern const uint8_t u8x8_font_7x14B_1x2_n[] U8X8_FONT_SECTION("u8x8_font_7x14B_1x2_n");
extern const uint8_t u8x8_font_open_iconic_arrow_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_1x1");
extern const uint8_t u8x8_font_open_iconic_check_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_1x1");
extern const uint8_t u8x8_font_open_iconic_embedded_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_1x1");
extern const uint8_t u8x8_font_open_iconic_play_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_1x1");
extern const uint8_t u8x8_font_open_iconic_thing_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_1x1");
extern const uint8_t u8x8_font_open_iconic_weather_1x1[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_1x1");
extern const uint8_t u8x8_font_open_iconic_arrow_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_2x2");
extern const uint8_t u8x8_font_open_iconic_check_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_2x2");
extern const uint8_t u8x8_font_open_iconic_embedded_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_2x2");
extern const uint8_t u8x8_font_open_iconic_play_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_2x2");
extern const uint8_t u8x8_font_open_iconic_thing_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_2x2");
extern const uint8_t u8x8_font_open_iconic_weather_2x2[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_2x2");
extern const uint8_t u8x8_font_open_iconic_arrow_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_4x4");
extern const uint8_t u8x8_font_open_iconic_check_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_4x4");
extern const uint8_t u8x8_font_open_iconic_embedded_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_4x4");
extern const uint8_t u8x8_font_open_iconic_play_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_4x4");
extern const uint8_t u8x8_font_open_iconic_thing_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_4x4");
extern const uint8_t u8x8_font_open_iconic_weather_4x4[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_4x4");
extern const uint8_t u8x8_font_open_iconic_arrow_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_arrow_8x8");
extern const uint8_t u8x8_font_open_iconic_check_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_check_8x8");
extern const uint8_t u8x8_font_open_iconic_embedded_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_embedded_8x8");
extern const uint8_t u8x8_font_open_iconic_play_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_play_8x8");
extern const uint8_t u8x8_font_open_iconic_thing_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_thing_8x8");
extern const uint8_t u8x8_font_open_iconic_weather_8x8[] U8X8_FONT_SECTION("u8x8_font_open_iconic_weather_8x8");
extern const uint8_t u8x8_font_profont29_2x3_f[] U8X8_FONT_SECTION("u8x8_font_profont29_2x3_f");
extern const uint8_t u8x8_font_profont29_2x3_r[] U8X8_FONT_SECTION("u8x8_font_profont29_2x3_r");
extern const uint8_t u8x8_font_profont29_2x3_n[] U8X8_FONT_SECTION("u8x8_font_profont29_2x3_n");
extern const uint8_t u8x8_font_artossans8_r[] U8X8_FONT_SECTION("u8x8_font_artossans8_r");
extern const uint8_t u8x8_font_artossans8_n[] U8X8_FONT_SECTION("u8x8_font_artossans8_n");
extern const uint8_t u8x8_font_artossans8_u[] U8X8_FONT_SECTION("u8x8_font_artossans8_u");
extern const uint8_t u8x8_font_artosserif8_r[] U8X8_FONT_SECTION("u8x8_font_artosserif8_r");
extern const uint8_t u8x8_font_artosserif8_n[] U8X8_FONT_SECTION("u8x8_font_artosserif8_n");
extern const uint8_t u8x8_font_artosserif8_u[] U8X8_FONT_SECTION("u8x8_font_artosserif8_u");
extern const uint8_t u8x8_font_chroma48medium8_r[] U8X8_FONT_SECTION("u8x8_font_chroma48medium8_r");
extern const uint8_t u8x8_font_chroma48medium8_n[] U8X8_FONT_SECTION("u8x8_font_chroma48medium8_n");
extern const uint8_t u8x8_font_chroma48medium8_u[] U8X8_FONT_SECTION("u8x8_font_chroma48medium8_u");
extern const uint8_t u8x8_font_saikyosansbold8_n[] U8X8_FONT_SECTION("u8x8_font_saikyosansbold8_n");
extern const uint8_t u8x8_font_saikyosansbold8_u[] U8X8_FONT_SECTION("u8x8_font_saikyosansbold8_u");
extern const uint8_t u8x8_font_torussansbold8_r[] U8X8_FONT_SECTION("u8x8_font_torussansbold8_r");
extern const uint8_t u8x8_font_torussansbold8_n[] U8X8_FONT_SECTION("u8x8_font_torussansbold8_n");
extern const uint8_t u8x8_font_torussansbold8_u[] U8X8_FONT_SECTION("u8x8_font_torussansbold8_u");
extern const uint8_t u8x8_font_victoriabold8_r[] U8X8_FONT_SECTION("u8x8_font_victoriabold8_r");
extern const uint8_t u8x8_font_victoriabold8_n[] U8X8_FONT_SECTION("u8x8_font_victoriabold8_n");
extern const uint8_t u8x8_font_victoriabold8_u[] U8X8_FONT_SECTION("u8x8_font_victoriabold8_u");
extern const uint8_t u8x8_font_victoriamedium8_r[] U8X8_FONT_SECTION("u8x8_font_victoriamedium8_r");
extern const uint8_t u8x8_font_victoriamedium8_n[] U8X8_FONT_SECTION("u8x8_font_victoriamedium8_n");
extern const uint8_t u8x8_font_victoriamedium8_u[] U8X8_FONT_SECTION("u8x8_font_victoriamedium8_u");
extern const uint8_t u8x8_font_courB18_2x3_f[] U8X8_FONT_SECTION("u8x8_font_courB18_2x3_f");
extern const uint8_t u8x8_font_courB18_2x3_r[] U8X8_FONT_SECTION("u8x8_font_courB18_2x3_r");
extern const uint8_t u8x8_font_courB18_2x3_n[] U8X8_FONT_SECTION("u8x8_font_courB18_2x3_n");
extern const uint8_t u8x8_font_courR18_2x3_f[] U8X8_FONT_SECTION("u8x8_font_courR18_2x3_f");
extern const uint8_t u8x8_font_courR18_2x3_r[] U8X8_FONT_SECTION("u8x8_font_courR18_2x3_r");
extern const uint8_t u8x8_font_courR18_2x3_n[] U8X8_FONT_SECTION("u8x8_font_courR18_2x3_n");
extern const uint8_t u8x8_font_courB24_3x4_f[] U8X8_FONT_SECTION("u8x8_font_courB24_3x4_f");
extern const uint8_t u8x8_font_courB24_3x4_r[] U8X8_FONT_SECTION("u8x8_font_courB24_3x4_r");
extern const uint8_t u8x8_font_courB24_3x4_n[] U8X8_FONT_SECTION("u8x8_font_courB24_3x4_n");
extern const uint8_t u8x8_font_courR24_3x4_f[] U8X8_FONT_SECTION("u8x8_font_courR24_3x4_f");
extern const uint8_t u8x8_font_courR24_3x4_r[] U8X8_FONT_SECTION("u8x8_font_courR24_3x4_r");
extern const uint8_t u8x8_font_courR24_3x4_n[] U8X8_FONT_SECTION("u8x8_font_courR24_3x4_n");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_o_2x2_f[] U8X8_FONT_SECTION("u8x8_font_lucasarts_scumm_subtitle_o_2x2_f");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_o_2x2_r[] U8X8_FONT_SECTION("u8x8_font_lucasarts_scumm_subtitle_o_2x2_r");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_o_2x2_n[] U8X8_FONT_SECTION("u8x8_font_lucasarts_scumm_subtitle_o_2x2_n");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_r_2x2_f[] U8X8_FONT_SECTION("u8x8_font_lucasarts_scumm_subtitle_r_2x2_f");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_r_2x2_r[] U8X8_FONT_SECTION("u8x8_font_lucasarts_scumm_subtitle_r_2x2_r");
extern const uint8_t u8x8_font_lucasarts_scumm_subtitle_r_2x2_n[] U8X8_FONT_SECTION("u8x8_font_lucasarts_scumm_subtitle_r_2x2_n");
extern const uint8_t u8x8_font_inr21_2x4_f[] U8X8_FONT_SECTION("u8x8_font_inr21_2x4_f");
extern const uint8_t u8x8_font_inr21_2x4_r[] U8X8_FONT_SECTION("u8x8_font_inr21_2x4_r");
extern const uint8_t u8x8_font_inr21_2x4_n[] U8X8_FONT_SECTION("u8x8_font_inr21_2x4_n");
extern const uint8_t u8x8_font_inr33_3x6_f[] U8X8_FONT_SECTION("u8x8_font_inr33_3x6_f");
extern const uint8_t u8x8_font_inr33_3x6_r[] U8X8_FONT_SECTION("u8x8_font_inr33_3x6_r");
extern const uint8_t u8x8_font_inr33_3x6_n[] U8X8_FONT_SECTION("u8x8_font_inr33_3x6_n");
extern const uint8_t u8x8_font_inr46_4x8_f[] U8X8_FONT_SECTION("u8x8_font_inr46_4x8_f");
extern const uint8_t u8x8_font_inr46_4x8_r[] U8X8_FONT_SECTION("u8x8_font_inr46_4x8_r");
extern const uint8_t u8x8_font_inr46_4x8_n[] U8X8_FONT_SECTION("u8x8_font_inr46_4x8_n");
extern const uint8_t u8x8_font_inb21_2x4_f[] U8X8_FONT_SECTION("u8x8_font_inb21_2x4_f");
extern const uint8_t u8x8_font_inb21_2x4_r[] U8X8_FONT_SECTION("u8x8_font_inb21_2x4_r");
extern const uint8_t u8x8_font_inb21_2x4_n[] U8X8_FONT_SECTION("u8x8_font_inb21_2x4_n");
extern const uint8_t u8x8_font_inb33_3x6_f[] U8X8_FONT_SECTION("u8x8_font_inb33_3x6_f");
extern const uint8_t u8x8_font_inb33_3x6_r[] U8X8_FONT_SECTION("u8x8_font_inb33_3x6_r");
extern const uint8_t u8x8_font_inb33_3x6_n[] U8X8_FONT_SECTION("u8x8_font_inb33_3x6_n");
extern const uint8_t u8x8_font_inb46_4x8_f[] U8X8_FONT_SECTION("u8x8_font_inb46_4x8_f");
extern const uint8_t u8x8_font_inb46_4x8_r[] U8X8_FONT_SECTION("u8x8_font_inb46_4x8_r");
extern const uint8_t u8x8_font_inb46_4x8_n[] U8X8_FONT_SECTION("u8x8_font_inb46_4x8_n");
extern const uint8_t u8x8_font_pressstart2p_f[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_f");
extern const uint8_t u8x8_font_pressstart2p_r[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_r");
extern const uint8_t u8x8_font_pressstart2p_n[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_n");
extern const uint8_t u8x8_font_pressstart2p_u[] U8X8_FONT_SECTION("u8x8_font_pressstart2p_u");
extern const uint8_t u8x8_font_pcsenior_f[] U8X8_FONT_SECTION("u8x8_font_pcsenior_f");
extern const uint8_t u8x8_font_pcsenior_r[] U8X8_FONT_SECTION("u8x8_font_pcsenior_r");
extern const uint8_t u8x8_font_pcsenior_n[] U8X8_FONT_SECTION("u8x8_font_pcsenior_n");
extern const uint8_t u8x8_font_pcsenior_u[] U8X8_FONT_SECTION("u8x8_font_pcsenior_u");
extern const uint8_t u8x8_font_pxplusibmcgathin_f[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_f");
extern const uint8_t u8x8_font_pxplusibmcgathin_r[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_r");
extern const uint8_t u8x8_font_pxplusibmcgathin_n[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_n");
extern const uint8_t u8x8_font_pxplusibmcgathin_u[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcgathin_u");
extern const uint8_t u8x8_font_pxplusibmcga_f[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_f");
extern const uint8_t u8x8_font_pxplusibmcga_r[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_r");
extern const uint8_t u8x8_font_pxplusibmcga_n[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_n");
extern const uint8_t u8x8_font_pxplusibmcga_u[] U8X8_FONT_SECTION("u8x8_font_pxplusibmcga_u");
extern const uint8_t u8x8_font_pxplustandynewtv_f[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_f");
extern const uint8_t u8x8_font_pxplustandynewtv_r[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_r");
extern const uint8_t u8x8_font_pxplustandynewtv_n[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_n");
extern const uint8_t u8x8_font_pxplustandynewtv_u[] U8X8_FONT_SECTION("u8x8_font_pxplustandynewtv_u");
extern const uint8_t u8x8_font_px437wyse700a_2x2_f[] U8X8_FONT_SECTION("u8x8_font_px437wyse700a_2x2_f");
extern const uint8_t u8x8_font_px437wyse700a_2x2_r[] U8X8_FONT_SECTION("u8x8_font_px437wyse700a_2x2_r");
extern const uint8_t u8x8_font_px437wyse700a_2x2_n[] U8X8_FONT_SECTION("u8x8_font_px437wyse700a_2x2_n");
extern const uint8_t u8x8_font_px437wyse700b_2x2_f[] U8X8_FONT_SECTION("u8x8_font_px437wyse700b_2x2_f");
extern const uint8_t u8x8_font_px437wyse700b_2x2_r[] U8X8_FONT_SECTION("u8x8_font_px437wyse700b_2x2_r");
extern const uint8_t u8x8_font_px437wyse700b_2x2_n[] U8X8_FONT_SECTION("u8x8_font_px437wyse700b_2x2_n");

/* end font list */


#ifdef __cplusplus
}
#endif


#endif  /* _U8X8_H */


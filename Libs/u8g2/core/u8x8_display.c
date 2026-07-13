/*
  
  u8x8_display.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

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
  
  
  Abstraction layer for the graphics controller.
  Main goal is the placement of a 8x8 pixel block (tile) on the display.
  
*/


#include "u8x8.h"

/*==========================================*/
/* internal library function */

/*
  这是 U8X8_MSG_DISPLAY_SETUP_MEMORY 函数的辅助函数。
  它可以在显示回调函数中调用，以执行通常的标准任务。
*/
void u8x8_d_helper_display_setup_memory(u8x8_t *u8x8, const u8x8_display_info_t *display_info)
{
      /* 1) 设置显示信息结构体 */
      u8x8->display_info = display_info;
      u8x8->x_offset = u8x8->display_info->default_x_offset;
}

/*
  这是 U8X8_MSG_DISPLAY_INIT 函数的辅助函数。
  它可以在显示回调函数中调用，以执行通常的标准任务。
*/
void u8x8_d_helper_display_init(u8x8_t *u8x8)
{
      /* 2) 将端口方向应用于GPIO线，并为IO线应用默认值 */
      u8x8_gpio_Init(u8x8);
      u8x8_cad_Init(u8x8);              /* 这也会调用 U8X8_MSG_BYTE_INIT, byte init 不会调用 GPIO_INIT */

      /* 3) 执行复位 */
      u8x8_gpio_SetReset(u8x8, 1);
      u8x8_gpio_Delay(u8x8, U8X8_MSG_DELAY_MILLI, u8x8->display_info->reset_pulse_width_ms);
      u8x8_gpio_SetReset(u8x8, 0);
      u8x8_gpio_Delay(u8x8, U8X8_MSG_DELAY_MILLI, u8x8->display_info->reset_pulse_width_ms);
      u8x8_gpio_SetReset(u8x8, 1);
      u8x8_gpio_Delay(u8x8, U8X8_MSG_DELAY_MILLI, u8x8->display_info->post_reset_wait_ms);
}    

/*==========================================*/
/* official functions */

uint8_t u8x8_DrawTile(u8x8_t *u8x8, uint8_t x, uint8_t y, uint8_t cnt, uint8_t *tile_ptr)
{
  u8x8_tile_t tile;
  tile.x_pos = x;
  tile.y_pos = y;
  tile.cnt = cnt;
  tile.tile_ptr = tile_ptr;
  return u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_DRAW_TILE, 1, (void *)&tile);
}

/* should be implemented as macro */
void u8x8_SetupMemory(u8x8_t *u8x8)
{
  u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_SETUP_MEMORY, 0, NULL);  
}

/*
  这只会初始化显示接口，与InitDisplay相比，它不会发出复位，也不会上传初始化代码。
  比较：
  调用                                                  u8x8_InitInterface              u8x8_InitDisplay
  初始化接口                                                是                                      是
  复位显示器                                                 否                                      是
  上传显示初始化代码                              否                                      是

  u8x8_InitInterface() 是 u8x8_InitDisplay() 的替代函数。不要同时调用两者。
*/
void u8x8_InitInterface(u8x8_t *u8x8)
{
  u8x8_gpio_Init(u8x8);
  u8x8_cad_Init(u8x8);              /* 这也会调用 U8X8_MSG_BYTE_INIT, byte init 不会调用 GPIO_INIT */
}

/*
  这将向显示器发送显示初始化消息。
  然后显示器本身将从上面调用 u8x8_d_helper_display_init()。这包括：
    GPIO初始化（设置端口方向）
    BYTE初始化（CAD初始化的一部分：可能会设置一些电平）
    CAD初始化（将设置诸如I2C默认地址之类的内容）
    向显示器发出复位：这通常会关闭显示器
  另外，每个显示器都会将初始化代码设置到显示器，这在大多数情况下也会关闭显示器（Arduino代码稍后禁用省电模式）

  实际上，这个过程最好称为InitInterfaceAndDisplay，因为它实际上两者都做。
  （实际上 u8x8_InitInterface() 不是直接调用的，而是 u8x8_gpio_Init 和 u8x8_cad_Init，
  而后者又由 u8x8_InitInterface() 调用）


  InitDisplay由Arduino的begin()函数调用

  在某些情况下，不需要初始化显示器（例如，如果显示器已经在运行，但控制器从深度睡眠模式中唤醒）。
  那么可以跳过InitDisplay，但需要执行 u8x8_InitInterface()（== u8x8_gpio_Init() 和 u8x8_cad_Init()）。

*/
void u8x8_InitDisplay(u8x8_t *u8x8)
{
  u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_INIT, 0, NULL);       /* 这将调用 u8x8_d_helper_display_init() 并将初始化序列发送到显示器 */
  /* u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_SET_FLIP_MODE, 0, NULL);  */ /* 在U8X8_MSG_DISPLAY_INIT之后调用flip mode 0是有意义的 */
}

void u8x8_SetPowerSave(u8x8_t *u8x8, uint8_t is_enable)
{
  u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_SET_POWER_SAVE, is_enable, NULL);  
}

void u8x8_SetFlipMode(u8x8_t *u8x8, uint8_t mode)
{
  u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_SET_FLIP_MODE, mode, NULL);  
}

void u8x8_SetContrast(u8x8_t *u8x8, uint8_t value)
{
  u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_SET_CONTRAST, value, NULL);  
}

void u8x8_RefreshDisplay(u8x8_t *u8x8)
{
  u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_REFRESH, 0, NULL);
  //++FPS_Count;
}

void u8x8_ClearDisplayWithTile(u8x8_t *u8x8, const uint8_t *buf)
{
  u8x8_tile_t tile;
  uint8_t h;

  tile.x_pos = 0;
  tile.cnt = 1;
  tile.tile_ptr = (uint8_t *)buf;		/* tile_ptr should be const, but isn't */
  
  h = u8x8->display_info->tile_height;
  tile.y_pos = 0;
  do
  {
    u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_DRAW_TILE, u8x8->display_info->tile_width, (void *)&tile);
    tile.y_pos++;
  } while( tile.y_pos < h );
}

void u8x8_ClearDisplay(u8x8_t *u8x8)
{
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  u8x8_ClearDisplayWithTile(u8x8, buf);
}

void u8x8_FillDisplay(u8x8_t *u8x8)
{
  uint8_t buf[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };
  u8x8_ClearDisplayWithTile(u8x8, buf);
}

void u8x8_ClearLine(u8x8_t *u8x8, uint8_t line)
{
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  u8x8_tile_t tile;
  if ( line < u8x8->display_info->tile_height )
  {
    tile.x_pos = 0;
    tile.y_pos = line;
    tile.cnt = 1;
    tile.tile_ptr = (uint8_t *)buf;		/* tile_ptr should be const, but isn't */
    u8x8->display_cb(u8x8, U8X8_MSG_DISPLAY_DRAW_TILE, u8x8->display_info->tile_width, (void *)&tile);
  }  
}
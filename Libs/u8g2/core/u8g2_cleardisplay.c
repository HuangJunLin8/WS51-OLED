/*

  u8g2_cleardisplay.c

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

*/
#include "u8g2.h"

/* 清除屏幕缓冲区和显示，对所有u8g2显示器都可靠。 */
/* 这是通过u8g2图片循环完成的，因为在所有情况下我们都不能使用u8x8函数。 */
void u8g2_ClearDisplay(u8g2_t *u8g2)
{
  u8g2_FirstPage(u8g2);
  do {
  } while ( u8g2_NextPage(u8g2) );
  /* 
    这个函数通常在启动时(u8g2.begin())调用。
    然而，用户可能希望在全缓冲模式下使用清除和发送命令。
    这将不起作用，因为当前的瓦片行被上面的图片循环修改了。
    为了解决这个问题，将瓦片行重置为0，问题 #370
    一个解决方法是用户手动将当前瓦片行设置为0。
  */
  u8g2_SetBufferCurrTileRow(u8g2, 0);  
}


/*****************************************************************************
* | File        :   LCD_1IN14_V2.h
* | Function    :   test Demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2021-03-16
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#ifndef __LCD_1IN14_V2_H
#define __LCD_1IN14_V2_H


#include "DEV_Config.h"
#include <stdint.h>

#include <stdlib.h>     //itoa()
#include <stdio.h>


#define LCD_1IN14_V2_HEIGHT 240
#define LCD_1IN14_V2_WIDTH 135


#define HORIZONTAL 1
#define VERTICAL   0

#define LCD_1IN14_V2_SetBacklight(Value) ; 

#define WHITE         0xFFFF
#define BLACK		  0x0000
#define BLUE 		  0x001F
#define BRED 	      0XF81F
#define GRED 		  0XFFE0
#define GBLUE		  0X07FF
#define RED  		  0xF800
#define MAGENTA		  0xF81F
#define GREEN		  0x07E0
#define CYAN 		  0x7FFF
#define YELLOW		  0xFFE0
#define BROWN		  0XBC40
#define BRRED		  0XFC07
#define GRAY 	      0X8430
#define DARKBLUE	  0X01CF
#define LIGHTBLUE	  0X7D7C
#define GRAYBLUE      0X5458
#define LIGHTGREEN    0X841F
#define LGRAY 		  0XC618
#define LGRAYBLUE     0XA651
#define LBBLUE        0X2B12

typedef struct{
    uint16_t WIDTH;
    uint16_t HEIGHT;
    uint8_t SCAN_DIR;
}LCD_1IN14_V2_ATTRIBUTES;
extern LCD_1IN14_V2_ATTRIBUTES LCD_1IN14_V2;

/********************************************************************************
function:	
			Macro definition variable name
********************************************************************************/
void LCD_1IN14_V2_Init(uint8_t Scan_dir);
void LCD_1IN14_V2_Clear(uint16_t Color);
void LCD_1IN14_V2_Display(uint16_t *Image);
void LCD_1IN14_V2_DisplayWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t *Image);
void LCD_1IN14_V2_DisplayPoint(uint16_t X, uint16_t Y, uint16_t Color);
void LCD_1IN14_V2_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend);
void Handler_1IN14_V2_LCD(int signo);
#endif

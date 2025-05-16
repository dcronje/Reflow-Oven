/*****************************************************************************
* | File      	:   LCD_1in8_LVGL_test.c
* | Author      :   Waveshare team
* | Function    :   1.8inch LCD LVGL demo
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2024-07-08
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
#include "LCD_Test.h"
#include "LCD_1in8.h"

static void Widgets_Init(lvgl_data_struct *dat);

/********************************************************************************
function:   Main function
parameter:
********************************************************************************/
int LCD_1in8_test(void)
{
    if(DEV_Module_Init()!=0){
        return -1;
    }
    
    /*LCD Init*/
    printf("1.8inch LCD LVGL demo...\r\n");
    LCD_1IN8_Init(HORIZONTAL);
    LCD_1IN8_Clear(WHITE);

    /*Config parameters*/
    LCD_SetWindows = LCD_1IN8_SetWindows;
    DISP_HOR_RES = LCD_1IN8_HEIGHT;
    DISP_VER_RES = LCD_1IN8_WIDTH;

    /*Init LVGL data structure*/    
    lvgl_data_struct *dat = (lvgl_data_struct *)malloc(sizeof(lvgl_data_struct));
    memset(dat->scr, 0, sizeof(dat->scr));
    
    /*Init LVGL*/
    LVGL_Init();
    Widgets_Init(dat);

    while(1)
    {
        lv_task_handler();
        DEV_Delay_ms(3000); 

        /*Switch to the next interface every 3 seconds*/
        switch_to_next_screen(dat->scr);
    }
    
    DEV_Module_Exit();
    return 0;
}

/********************************************************************************
function:   Initialize Widgets
parameter:
********************************************************************************/
static void Widgets_Init(lvgl_data_struct *dat)
{
    /*Screen1: Just a picture*/
    dat->scr[0] = lv_obj_create(NULL);
    
    /*Declare and load the image resource*/
    LV_IMG_DECLARE(LCD_1inch8);
    lv_obj_t *img1 = lv_img_create(dat->scr[0]);
    lv_img_set_src(img1, &LCD_1inch8);

    /*Align the image to the center of the screen*/
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);

    /*Load the first screen as the current active screen*/
    lv_scr_load(dat->scr[0]);

    /*Screen2: Just a picture*/
    dat->scr[1] = lv_obj_create(NULL);
    
    /*Declare and load the image resource*/
    LV_IMG_DECLARE(LCD_1inch8_2);
    lv_obj_t *img2 = lv_img_create(dat->scr[1]);
    lv_img_set_src(img2, &LCD_1inch8_2);
    lv_obj_align(img2, LV_ALIGN_CENTER, 0, 0);
}
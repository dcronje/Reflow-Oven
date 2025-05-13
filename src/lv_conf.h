#pragma once

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Basic configuration */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0


#define LV_USE_DRAW_SW 1
#define LV_USE_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE

#define LV_USE_DRAW_ARM2D 0
#define LV_USE_DRAW_ARM2D_SYNC 0
#define LV_USE_DRAW_HELIUM 0
#define LV_USE_DRAW_ARM_HELIUM 0
#define LV_USE_DRAW_NATIVE_HELIUM_ASM 0

/* Disable all ARM-specific optimizations for Pico */
#define LV_USE_HELIUM_ASM 0
#define LV_USE_ARM_HELIUM 0
#define LV_USE_ARM_NEON 0
#define LV_USE_ARM_DSP 0
#define LV_USE_NATIVE_HELIUM_ASM 0

#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_INFO
#define LV_LOG_PRINTF 1

/* Memory settings */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (256U * 1024U)  // 256KB for LVGL - increased for Pico 2's 520KB SRAM
#define LV_MEM_ATTR
#define LV_MEM_ADR 0
#define LV_MEM_AUTO_DEFRAG 1

/* HAL settings */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "pico/time.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR ((uint32_t)time_us_64() / 1000)

/* Feature configuration */
#define LV_USE_ANIMATION 1
#define LV_USE_SHADOW 1
#define LV_USE_BLEND_MODES 1
#define LV_USE_OPACITY 1
#define LV_USE_IMG_TRANSFORM 1
#define LV_USE_IMG_CACHE 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_SLIDER 1
#define LV_USE_TABLE 1
#define LV_USE_CHART 1
#define LV_USE_TEXTAREA 1
#define LV_USE_SPINBOX 1
#define LV_USE_SPINNER 1
#define LV_USE_CALENDAR 1
#define LV_USE_CANVAS 1
#define LV_USE_API_EXTENSION_V6 1
#define LV_USE_API_EXTENSION_V7 1

/* Display driver */
#define LV_USE_ST7789 1
#define LV_USE_GPU 0
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL 0
#define LV_HOR_RES_MAX 240
#define LV_VER_RES_MAX 320
#define LV_DPI 130

/* Input device settings */
#define LV_USE_INDEV_TOUCHPAD 1
#define LV_USE_INDEV_ENCODER 1
#define LV_USE_INDEV_BUTTON 1

/* Theme settings */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

/* Font settings */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Widget settings */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMG 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_TABLE 1

/* Extra settings */
#define LV_USE_USER_DATA 1
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1
#define LV_USE_ASSERT_STR 1
#define LV_USE_ASSERT_OBJ 1
#define LV_USE_ASSERT_STYLE 1

/* File system */
#define LV_USE_FS_STDIO 0
// #define LV_FS_STDIO_LETTER 'A'

/* Widget usage */
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_LINE 1
#define LV_USE_ROLLER 1
#define LV_USE_SWITCH 1

/* Theme usage */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

/* Font usage */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_38 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_42 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_46 1
#define LV_FONT_MONTSERRAT_48 1

/* File system */
// #define LV_USE_FS_STDIO 1
// #define LV_FS_STDIO_LETTER 'A'

/* Misc */
#define LV_USE_USER_DATA 1
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1
#define LV_USE_ASSERT_STR 1
#define LV_USE_ASSERT_OBJ 1
#define LV_USE_ASSERT_STYLE 1

#endif /*LV_CONF_H*/ 
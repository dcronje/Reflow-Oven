#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*--------------------
 * COLOR SETTINGS
 *-------------------*/
// Use 16-bit color depth (RGB565)
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1                // Swap bytes if your display uses BGR instead of RGB (0 = no swap)
#define LV_COLOR_SCREEN_TRANSP 0          // Enable screen transparency (off to save memory)

/*--------------------
 * OS INTEGRATION
 *-------------------*/
#define LV_USE_OS LV_OS_FREERTOS          // Use FreeRTOS as the OS backend

/*--------------------
 * DRAW ENGINES
 *-------------------*/
#define LV_USE_DRAW 1                     // Enable software drawing engine
#define LV_USE_DRAW_SW 1                 // Use the standard C drawing engine

// Disable all ARM-specific hardware acceleration (RP2040 has no Helium or DSP support)
#define LV_USE_DRAW_ARM2D 0
#define LV_USE_DRAW_ARM2D_SYNC 0
#define LV_USE_DRAW_HELIUM 0
#define LV_USE_ARM_HELIUM 0
#define LV_USE_ARM_DSP 0
#define LV_USE_NATIVE_HELIUM_ASM 0

/*--------------------
 * LOGGING
 *-------------------*/
#define LV_USE_LOG 1                      // Enable internal logging
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN //LV_LOG_LEVEL_TRACE   // Show warnings and errors
#define LV_LOG_PRINTF 1                  // Log using printf (enable if stdio is available)
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1
#define LV_USE_ASSERT_OBJ 1
/*--------------------
 * MEMORY SETTINGS
 *-------------------*/
#define LV_MEM_CUSTOM 0                  // Use LVGL's built-in malloc (disable if you want to use FreeRTOS heap)
#define LV_MEM_SIZE (256U * 1024U)       // Allocate 256KB for LVGL (RP2040 RP2350 has ~520KB total)
#define LV_MEM_ATTR                      // Leave empty unless placing memory in a specific section
#define LV_MEM_ADR 0                     // Let LVGL allocate internally
#define LV_MEM_AUTO_DEFRAG 1             // Enable auto-defragmentation of heap (slight performance cost)

/*--------------------
 * TICK CONFIGURATION
 *-------------------*/
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "pico/time.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR ((uint32_t)time_us_64() / 1000)

/*--------------------
 * DISPLAY SETTINGS
 *-------------------*/
#define LV_USE_DISPLAY 1
#define LV_USE_DISP_RENDER_MODE_PARTIAL 1  // Use partial rendering (important for low RAM)
#define LV_HOR_RES_MAX 240                 // Horizontal resolution
#define LV_VER_RES_MAX 320                 // Vertical resolution
#define LV_DPI 130                         // Approximate pixel density

/*--------------------
 * FONT SETTINGS
 *-------------------*/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_USE_FONT_SUBPX 0                // Subpixel rendering (disabled, not useful on RGB565)
#define LV_FONT_SUBPX_BGR 0

/*--------------------
 * UI FEATURES
 *-------------------*/
#define LV_USE_ANIMATION 1
#define LV_USE_SHADOW 1
#define LV_USE_BLEND_MODES 1
#define LV_USE_OPACITY 1
#define LV_USE_IMG_TRANSFORM 1
#define LV_USE_IMG_CACHE 1
#define LV_USE_API_EXTENSION_V6 1
#define LV_USE_API_EXTENSION_V7 1
#define LV_USE_USER_DATA 1

/*--------------------
 * INPUT SUPPORT
 *-------------------*/
#define LV_USE_INDEV_ENCODER 1
#define LV_USE_INDEV_BUTTON 1
#define LV_USE_INDEV_TOUCHPAD 0           // Set to 1 if using a capacitive/resistive touchscreen

/*--------------------
 * THEME SETTINGS
 *-------------------*/
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

/*--------------------
 * WIDGET SUPPORT
 *-------------------*/
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
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
#define LV_USE_CHART 1
#define LV_USE_SPINBOX 1
#define LV_USE_SPINNER 1
#define LV_USE_CALENDAR 1

/*--------------------
 * FILESYSTEM (OPTIONAL)
 *-------------------*/
#define LV_USE_FS_STDIO 0  // Enable if using file-based assets via stdio
//#define LV_FS_STDIO_LETTER 'A'

/*--------------------
 * BASE UTILITIES
 *-------------------*/
#define LV_USE_TIMER 1
#define LV_USE_BASE64 1
#define LV_USE_STDLIB_MALLOC 1            // Only if LV_MEM_CUSTOM is 0

/*--------------------
 * ASSERTIONS
 *-------------------*/
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1
#define LV_USE_ASSERT_STR 1
#define LV_USE_ASSERT_OBJ 1
#define LV_USE_ASSERT_STYLE 1

#endif // LV_CONF_H
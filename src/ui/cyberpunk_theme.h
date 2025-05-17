// cyberpunk_theme.h
#pragma once

#include "lvgl.h"

// Define theme colors
#define CYBER_COLOR_BG        lv_color_hex(0x0A0A0A)
#define CYBER_COLOR_ACCENT    lv_color_hex(0xFFD500)
#define CYBER_COLOR_WARNING   lv_color_hex(0xFF3C00)
#define CYBER_COLOR_TEXT      lv_color_hex(0xFFFFFF)
#define CYBER_COLOR_BUTTON    lv_color_hex(0x202020)
#define CYBER_COLOR_HOVER     lv_color_hex(0xFF9500)

namespace CyberpunkTheme {

void init();

lv_style_t* getPanelStyle();
lv_style_t* getLabelStyle();
lv_style_t* getWarningBoxStyle();
lv_style_t* getMenuButtonStyle();
lv_style_t* getMenuButtonFocusedStyle();
lv_style_t* getTitleLabelStyle();

lv_obj_t* createStripedTitleLabel(lv_obj_t* parent, const char* text, int width, int stripeThickness = 12);
lv_obj_t* createStripedTitleLabel(lv_obj_t* parent, const char* text);
lv_obj_t* createCyberpunkButton(lv_obj_t* parent, const char* mainText, const char* edgeLabel, bool selected);;

} // namespace CyberpunkTheme

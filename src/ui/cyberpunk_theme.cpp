// cyberpunk_theme.cpp
#include "cyberpunk_theme.h"
#include "constants.h"

namespace CyberpunkTheme {

static lv_style_t stylePanel;
static lv_style_t styleLabel;
static lv_style_t styleWarningBox;
static lv_style_t styleMenuButton;
static lv_style_t styleMenuButtonFocused;
static lv_style_t styleTitleLabel;

void init() {
    lv_style_init(&stylePanel);
    lv_style_set_bg_color(&stylePanel, CYBER_COLOR_BG);
    lv_style_set_border_color(&stylePanel, CYBER_COLOR_ACCENT);
    lv_style_set_border_width(&stylePanel, 4);
    lv_style_set_radius(&stylePanel, 6);
    lv_style_set_pad_all(&stylePanel, 8);

    lv_style_init(&styleLabel);
    lv_style_set_text_color(&styleLabel, CYBER_COLOR_TEXT);
    lv_style_set_text_font(&styleLabel, &lv_font_montserrat_14);

    lv_style_init(&styleWarningBox);
    lv_style_set_bg_color(&styleWarningBox, CYBER_COLOR_WARNING);
    lv_style_set_border_color(&styleWarningBox, lv_color_black());
    lv_style_set_border_width(&styleWarningBox, 2);
    lv_style_set_radius(&styleWarningBox, 0);
    lv_style_set_pad_all(&styleWarningBox, 6);

    lv_style_init(&styleMenuButton);
    lv_style_set_bg_color(&styleMenuButton, CYBER_COLOR_BG); // black
    lv_style_set_border_color(&styleMenuButton, CYBER_COLOR_ACCENT); // yellow border
    lv_style_set_border_width(&styleMenuButton, 2);
    lv_style_set_radius(&styleMenuButton, 0);
    lv_style_set_pad_all(&styleMenuButton, 10);
    lv_style_set_text_color(&styleMenuButton, CYBER_COLOR_ACCENT); // yellow text
    lv_style_set_text_font(&styleMenuButton, &lv_font_montserrat_20);

    lv_style_init(&styleMenuButtonFocused);
    lv_style_set_bg_color(&styleMenuButtonFocused, CYBER_COLOR_ACCENT); // yellow bg
    lv_style_set_border_color(&styleMenuButtonFocused, CYBER_COLOR_BG); // black border
    lv_style_set_border_width(&styleMenuButtonFocused, 2);
    lv_style_set_radius(&styleMenuButtonFocused, 0);
    lv_style_set_pad_all(&styleMenuButtonFocused, 10);
    lv_style_set_text_color(&styleMenuButtonFocused, CYBER_COLOR_BG); // black text
    lv_style_set_text_font(&styleMenuButtonFocused, &lv_font_montserrat_20);

    lv_style_init(&styleTitleLabel);
    lv_style_set_text_color(&styleTitleLabel, CYBER_COLOR_ACCENT);
    lv_style_set_text_font(&styleTitleLabel, &lv_font_montserrat_20);
    lv_style_set_text_letter_space(&styleTitleLabel, 2);
    lv_style_set_pad_bottom(&styleTitleLabel, 10);
    lv_style_set_bg_color(&styleTitleLabel, CYBER_COLOR_BG);
    lv_style_set_bg_opa(&styleTitleLabel, LV_OPA_COVER);
    lv_style_set_border_side(&styleTitleLabel, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&styleTitleLabel, CYBER_COLOR_ACCENT);
    lv_style_set_border_width(&styleTitleLabel, 2);
}

lv_style_t* getPanelStyle() {
    return &stylePanel;
}

lv_style_t* getLabelStyle() {
    return &styleLabel;
}

lv_style_t* getWarningBoxStyle() {
    return &styleWarningBox;
}

lv_style_t* getMenuButtonStyle() {
    return &styleMenuButton;
}

lv_style_t* getMenuButtonFocusedStyle() {
    return &styleMenuButtonFocused;
}

lv_style_t* getTitleLabelStyle() {
    return &styleTitleLabel;
}

lv_obj_t* createStripedTitleLabel(lv_obj_t* parent, const char* text, int width) {
    const int height = 40;
    const int stripeSpacing = 20;
    const int stripeWidth = 12;
    const int stripeOvershoot = height / 2;

    static lv_color_t bg_buf[DISPLAY_WIDTH * 60];
    lv_obj_t* canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas, bg_buf, width, height, LV_COLOR_FORMAT_NATIVE);

    // 🔥 This line is crucial to avoid extra spacing
    lv_obj_set_size(canvas, width, height);

    // ✅ Add: Prevent layout expansion caused by children
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);  // Prevent layout flag issues
    lv_obj_set_scrollbar_mode(canvas, LV_SCROLLBAR_MODE_OFF); // Ensure no scrollbars
    lv_obj_set_style_max_height(canvas, height, 0);      // Cap maximum height
    lv_obj_set_style_min_height(canvas, height, 0);      // Force minimum height
    lv_obj_set_style_clip_corner(canvas, true, 0);       // Clip overflow

    // Cleanup styling
    lv_obj_set_style_pad_all(canvas, 0, 0);
    lv_obj_set_style_margin_all(canvas, 0, 0);
    lv_obj_set_style_border_width(canvas, 0, 0);

    lv_canvas_fill_bg(canvas, CYBER_COLOR_ACCENT, LV_OPA_COVER);

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    for (int x = -40; x < width; x += stripeSpacing) {
        lv_draw_line_dsc_t line_dsc;
        lv_draw_line_dsc_init(&line_dsc);
        line_dsc.color = lv_color_black();
        line_dsc.width = stripeWidth;

        line_dsc.p1.x = x;
        line_dsc.p1.y = -stripeOvershoot;
        line_dsc.p2.x = x + height;
        line_dsc.p2.y = height + stripeOvershoot;
        lv_draw_line(&layer, &line_dsc);
    }

    lv_canvas_finish_layer(canvas, &layer);

    const int outlineOffset = 1;
    lv_point_t offsets[8] = {
        {-outlineOffset, -outlineOffset},
        { 0,             -outlineOffset},
        { outlineOffset, -outlineOffset},
        {-outlineOffset,  0},
        { outlineOffset,  0},
        {-outlineOffset,  outlineOffset},
        { 0,              outlineOffset},
        { outlineOffset,  outlineOffset}
    };

    for (int i = 0; i < 8; ++i) {
        lv_obj_t* shadow = lv_label_create(canvas);
        lv_label_set_text(shadow, text);
        lv_obj_set_style_text_color(shadow, lv_color_black(), 0);
        lv_obj_set_style_text_font(shadow, &lv_font_montserrat_20, 0);
        lv_obj_align(shadow, LV_ALIGN_CENTER, offsets[i].x, offsets[i].y);

        // ✅ Add: Constrain child label height
        lv_obj_set_style_pad_all(shadow, 0, 0);
        lv_obj_set_style_margin_all(shadow, 0, 0);
    }

    lv_obj_t* label = lv_label_create(canvas);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);

    // ✅ Add: Constrain main label height
    lv_obj_set_style_pad_all(label, 0, 0);
    lv_obj_set_style_margin_all(label, 0, 0);

    return canvas;
}

lv_obj_t* createStripedTitleLabel(lv_obj_t* parent, const char* text) {
    return createStripedTitleLabel(parent, text, DISPLAY_WIDTH);
}

lv_obj_t* createMenuButton(lv_obj_t* parent, const char* text, bool isFocused, int index, int total, int width, int height) {
    // Simple buffer allocation - no macro needed in LVGL 9
    static lv_color_t buf[DISPLAY_WIDTH * 60];

    lv_obj_t* canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas, buf, width, height, LV_COLOR_FORMAT_NATIVE);
    lv_obj_set_style_border_width(canvas, 0, 0);
    lv_obj_set_style_pad_all(canvas, 0, 0);
    lv_obj_set_scrollbar_mode(canvas, LV_SCROLLBAR_MODE_OFF);

    lv_color_t bg = isFocused ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG;
    lv_canvas_fill_bg(canvas, bg, LV_OPA_COVER);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = bg;
    dsc.border_color = isFocused ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;
    dsc.border_width = 2;
    dsc.radius = 0;

    lv_area_t rect = { .x1 = 0, .y1 = 0, .x2 = width - 1, .y2 = height - 1 };
    
    // Initialize layer for drawing
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    
    // Draw rectangle
    lv_draw_rect(&layer, &dsc, &rect);
    
    // Draw diagonal line in the same layer
    lv_draw_line_dsc_t cut;
    lv_draw_line_dsc_init(&cut);
    cut.color = isFocused ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG;
    cut.width = 2;
    
    // Set line points directly in descriptor
    cut.p1.x = 0;
    cut.p1.y = height - 1;
    cut.p2.x = 15;
    cut.p2.y = height - 16;
    
    // Draw line
    lv_draw_line(&layer, &cut);
    
    // Finish the layer
    lv_canvas_finish_layer(canvas, &layer);

    lv_obj_t* label = lv_label_create(canvas);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, isFocused ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);

    char tag[12];
    snprintf(tag, sizeof(tag), "%02d/%02d", index + 1, total);
    lv_obj_t* tagLabel = lv_label_create(canvas);
    lv_label_set_text(tagLabel, tag);
    lv_obj_set_style_text_font(tagLabel, &lv_font_montserrat_14, 0); // Use font_14 instead of font_12
    lv_obj_set_style_text_color(tagLabel, isFocused ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT, 0);
    lv_obj_align(tagLabel, LV_ALIGN_BOTTOM_RIGHT, -8, -4);

    return canvas;
}

lv_obj_t* createMenuButton(lv_obj_t* parent, const char* text, bool isFocused, int index, int total) {
    return createMenuButton(parent, text, isFocused, index, total, DISPLAY_WIDTH - 20, 60);
}


} // namespace CyberpunkTheme
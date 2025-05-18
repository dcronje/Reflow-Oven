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

lv_obj_t* createStripedTitleLabel(lv_obj_t* parent, const char* text, int width, int stripeThickness,
                               lv_color_t textColor, lv_color_t textOutlineColor, int textOutlineThickness) {
    const int height = 40;
    const int stripeWidth = stripeThickness;        // Width of black diagonal stripes
    const int yellowWidth = stripeThickness * 0.7;  // Width of yellow areas between stripes
    const int stripeSpacing = stripeWidth + yellowWidth;
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

    // Create text outline using multiple shadow labels
    const int outlineOffset = textOutlineThickness;
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
        lv_obj_set_style_text_color(shadow, textOutlineColor, 0);
        lv_obj_set_style_text_font(shadow, &lv_font_montserrat_20, 0);
        lv_obj_align(shadow, LV_ALIGN_CENTER, offsets[i].x, offsets[i].y);

        // ✅ Add: Constrain child label height
        lv_obj_set_style_pad_all(shadow, 0, 0);
        lv_obj_set_style_margin_all(shadow, 0, 0);
    }

    lv_obj_t* label = lv_label_create(canvas);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, textColor, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);

    // ✅ Add: Constrain main label height
    lv_obj_set_style_pad_all(label, 0, 0);
    lv_obj_set_style_margin_all(label, 0, 0);

    return canvas;
}

lv_obj_t* createStripedTitleLabel(lv_obj_t* parent, const char* text) {
    return createStripedTitleLabel(parent, text, DISPLAY_WIDTH, 12, lv_color_white(), lv_color_black(), 1);
}

lv_obj_t* createCyberpunkButton(lv_obj_t* parent, const char* mainText, const char* edgeLabel, bool selected) {
    const int buttonWidth = 300;
    lv_obj_t* btn = lv_obj_create(parent);
    lv_obj_set_size(btn, buttonWidth, 50);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    // Style configuration
    lv_color_t bgColor     = selected ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG;
    lv_color_t textColor   = selected ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;
    lv_color_t borderColor = selected ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;

    // Apply main button styles
    lv_obj_set_style_bg_color(btn, bgColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, borderColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);

    // Main label (centered text)
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, mainText);
    lv_obj_set_style_text_color(label, textColor, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(label);

    // Edge label (bottom-right corner)
    lv_obj_t* corner = lv_label_create(btn);
    lv_label_set_text(corner, edgeLabel);
    lv_obj_set_style_text_color(corner, textColor, LV_PART_MAIN);
    lv_obj_set_style_text_font(corner, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(corner, LV_ALIGN_BOTTOM_RIGHT, -5, -3);

    return btn;
}

} // namespace CyberpunkTheme
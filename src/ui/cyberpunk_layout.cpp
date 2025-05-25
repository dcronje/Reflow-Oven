#include "lvgl.h"
#include "cyberpunk_layout.h"
#include "ui/cyberpunk_theme.h"
#include "constants.h"

// Animation durations and delays
constexpr uint32_t ANIM_DURATION = 300;
constexpr uint32_t ANIM_DELAY = 0;
constexpr uint32_t TAG_PRESS_DURATION = 200;

// Event callback for button press/release
CyberpunkLayout::CyberpunkLayout(lv_obj_t* parent) {
    // --- Main container using grid layout ---
    // This container fills the entire screen and contains 2 columns and 2 rows:
    // Columns: [Main Content] [Encoder Tag]
    // Rows:    [Main Content] [Bottom Tags]

    container = lv_obj_create(parent);

    // Column definitions:
    // - First column: flexible (takes all remaining width)
    // - Second column: sized to content (encoder tag)
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    // Row definitions:
    // - First row: flexible (main content)
    // - Second row: fixed height (bottom tag bar)
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), 40, LV_GRID_TEMPLATE_LAST};

    // Apply grid to container
    lv_obj_set_grid_dsc_array(container, col_dsc, row_dsc);
    lv_obj_set_layout(container, LV_LAYOUT_GRID);
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_bg_color(container, CYBER_COLOR_BG, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_radius(container, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    // --- Content Area (top-left grid cell) ---
    contentArea = lv_obj_create(container);
    lv_obj_set_grid_cell(contentArea, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_style_bg_color(contentArea, CYBER_COLOR_BG, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);
    lv_obj_set_style_radius(contentArea, 0, 0);
    lv_obj_set_style_pad_all(contentArea, 10, 0); // Inner padding for content widgets
    lv_obj_clear_flag(contentArea, LV_OBJ_FLAG_SCROLLABLE);

    // --- Bottom Tag Bar (bottom-left grid cell) ---
    bottomBar = lv_obj_create(container);
    lv_obj_set_grid_cell(bottomBar, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_style_bg_color(bottomBar, CYBER_COLOR_BG, 0);
    lv_obj_set_style_border_width(bottomBar, 0, 0);
    lv_obj_set_style_radius(bottomBar, 0, 0);
    lv_obj_set_style_pad_all(bottomBar, 0, 0);
    lv_obj_clear_flag(bottomBar, LV_OBJ_FLAG_SCROLLABLE);

    // Use horizontal flexbox layout for bottom tags
    lv_obj_set_layout(bottomBar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bottomBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottomBar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(bottomBar, 0, 0);
    // --- Create Bottom Tag Buttons ---
    for (int i = 0; i < 3; ++i) {
        lv_obj_t* btn = lv_btn_create(bottomBar);
        lv_obj_set_height(btn, 40);                 // Fixed height
        lv_obj_set_flex_grow(btn, 1);               // Let it grow evenly to fill the space
        lv_obj_set_style_bg_color(btn, CYBER_COLOR_BG, 0);
        lv_obj_set_style_border_color(btn, CYBER_COLOR_ACCENT, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 0, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

        // Label inside button
        lv_obj_t* label = lv_label_create(btn);
        lv_obj_set_style_text_color(label, CYBER_COLOR_ACCENT, 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_letter_space(label, -1, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(label); // Center inside the button

        tagContainers.push_back(btn);
        tagLabels.push_back(label);
    }

    // --- Encoder Tag Column (Right Side) ---
    encoderTagContainer = lv_obj_create(container);
        lv_obj_set_grid_cell(
        encoderTagContainer,
        LV_GRID_ALIGN_STRETCH, 1, 1,  // Column 1, span 1 column
        LV_GRID_ALIGN_STRETCH, 0, 1   // Row 0 only (same as content area)
    );
    lv_obj_set_width(encoderTagContainer, 30); // Narrow vertical strip
    lv_obj_set_style_bg_color(encoderTagContainer, CYBER_COLOR_BG, 0);
    lv_obj_set_style_border_color(encoderTagContainer, CYBER_COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(encoderTagContainer, 1, 0);
    lv_obj_set_style_radius(encoderTagContainer, 0, 0);
    
    lv_obj_clear_flag(encoderTagContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(encoderTagContainer, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(encoderTagContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(encoderTagContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_all(encoderTagContainer, 0, 0);
    lv_obj_set_style_pad_top(encoderTagContainer, 0, 0);
    lv_obj_set_style_pad_bottom(encoderTagContainer, 0, 0);
    lv_obj_set_style_pad_left(encoderTagContainer, -40, 0);
    lv_obj_set_style_pad_right(encoderTagContainer, 0, 0);

    lv_obj_set_style_margin_all(encoderTagContainer, 0, 0);
    lv_obj_set_style_margin_top(encoderTagContainer, 0, 0);
    lv_obj_set_style_margin_bottom(encoderTagContainer, 0, 0);

    lv_obj_set_style_pad_row(encoderTagContainer, 0, 0);
    lv_obj_set_style_pad_column(encoderTagContainer, 0, 0);

    // --- Default Encoder Label Text Rendered Vertically ---
    std::string defaultText = "MENU";
    for (char c : defaultText) {
        std::string s(1, c);
        lv_obj_t* chLabel = lv_label_create(encoderTagContainer);
        lv_obj_remove_style_all(chLabel);
        lv_label_set_text(chLabel, s.c_str());
        
        lv_obj_set_style_text_align(chLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(chLabel, LV_LABEL_LONG_CLIP);  // Prevent wrapping
        lv_obj_set_align(chLabel, LV_ALIGN_CENTER); 

        // Force label to a fixed size
        lv_obj_set_width(chLabel, 20);
        lv_obj_set_height(chLabel, LV_SIZE_CONTENT);

        // Absolute zero spacing
        lv_obj_set_style_pad_all(chLabel, 0, 0);
        lv_obj_set_style_pad_top(chLabel, 0, 0);
        lv_obj_set_style_pad_bottom(chLabel, 0, 0);
        lv_obj_set_style_pad_left(chLabel, 0, 0);
        lv_obj_set_style_pad_right(chLabel, 0, 0);

        lv_obj_set_style_margin_all(chLabel, 0, 0);
        lv_obj_set_style_margin_top(chLabel, 0, 0);
        lv_obj_set_style_margin_bottom(chLabel, 0, 0);
        lv_obj_set_style_margin_left(chLabel, 0, 0);
        lv_obj_set_style_margin_right(chLabel, 0, 0);

        // Text formatting
        lv_obj_set_style_text_align(chLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(chLabel, CYBER_COLOR_ACCENT, 0);
        lv_obj_set_style_text_font(chLabel, &lv_font_montserrat_14, 0);

        // Optional debug border to confirm spacing visually
        // lv_obj_set_style_border_width(chLabel, 1, 0);
        // lv_obj_set_style_border_color(chLabel, lv_color_hex(0xFF00FF), 0);

        encoderCharLabels.push_back(chLabel);
    }


    // Make tags visible initially
    setBottomTagsVisible(true);
    setEncoderTagVisible(true);
}




CyberpunkLayout::~CyberpunkLayout() {
    if (container) lv_obj_del(container);
}

void CyberpunkLayout::setBottomTags(const std::vector<std::string>& tags) {
    // Update existing tag labels
    for(size_t i = 0; i < std::min(tags.size(), size_t(3)); i++) {
        if(i < tagLabels.size() && tagLabels[i]) {
            lv_label_set_text(tagLabels[i], tags[i].c_str());
            lv_obj_clear_flag(tagContainers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // Hide any unused containers
    for(size_t i = tags.size(); i < 3; i++) {
        if(i < tagContainers.size()) {
            lv_obj_add_flag(tagContainers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void CyberpunkLayout::setBottomTagsVisible(bool visible) {
    for(auto& container : tagContainers) {
        if (container == nullptr) continue;
        if(visible) {
            lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(container, LV_OPA_COVER, 0);
        } else {
            lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(container, 0, 0);
        }
    }
}

void CyberpunkLayout::setEncoderTag(const std::string& tag, bool showScrollIcon) {
    std::string display = tag;
    if (showScrollIcon) {
        display += " ↑↓";  // You can replace this with simpler characters if needed
    }

    // Clear existing labels
    for (lv_obj_t* label : encoderCharLabels) {
        if (label) {
            lv_obj_del(label);
        }
    }
    encoderCharLabels.clear();

    // Create a new label per character
    for (char c : display) {
        std::string ch(1, c);
        lv_obj_t* chLabel = lv_label_create(encoderTagContainer);
        lv_label_set_text(chLabel, ch.c_str());
        lv_obj_set_style_text_color(chLabel, CYBER_COLOR_ACCENT, 0);
        lv_obj_set_style_text_font(chLabel, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_align(chLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_clear_flag(chLabel, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(chLabel, 0, 0);
        lv_obj_set_width(chLabel, LV_SIZE_CONTENT);
        encoderCharLabels.push_back(chLabel);
    }

    // Re-align the container just in case
    lv_obj_mark_layout_as_dirty(encoderTagContainer);
}


void CyberpunkLayout::setEncoderTagVisible(bool visible) {
    if (!encoderTagContainer) return;

    for (lv_obj_t* label : encoderCharLabels) {
        if (!label) continue;
        if (visible) {
            lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_translate_x(label, 20, 0);
            lv_obj_set_style_opa(label, 0, 0);

            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, label);
            lv_anim_set_values(&a, 20, 0);
            lv_anim_set_time(&a, ANIM_DURATION);
            lv_anim_set_delay(&a, ANIM_DELAY);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_translate_x);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_start(&a);

            lv_anim_t b;
            lv_anim_init(&b);
            lv_anim_set_var(&b, label);
            lv_anim_set_values(&b, 0, LV_OPA_COVER);
            lv_anim_set_time(&b, ANIM_DURATION);
            lv_anim_set_delay(&b, ANIM_DELAY);
            lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
            lv_anim_set_path_cb(&b, lv_anim_path_ease_out);
            lv_anim_start(&b);
        } else {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, label);
            lv_anim_set_values(&a, 0, 20);
            lv_anim_set_time(&a, ANIM_DURATION);
            lv_anim_set_delay(&a, ANIM_DELAY);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_translate_x);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
            lv_anim_set_ready_cb(&a, [](lv_anim_t* a) {
                lv_obj_add_flag((lv_obj_t*)a->var, LV_OBJ_FLAG_HIDDEN);
            });
            lv_anim_start(&a);

            lv_anim_t b;
            lv_anim_init(&b);
            lv_anim_set_var(&b, label);
            lv_anim_set_values(&b, LV_OPA_COVER, 0);
            lv_anim_set_time(&b, ANIM_DURATION);
            lv_anim_set_delay(&b, ANIM_DELAY);
            lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
            lv_anim_set_path_cb(&b, lv_anim_path_ease_in);
            lv_anim_start(&b);
        }
    }
}


void CyberpunkLayout::setTagPressed(size_t index, bool pressed, uint32_t duration_ms) {
    if (index >= tagContainers.size()) return;

    lv_obj_t* btn = tagContainers[index];
    if (!btn) return;

    lv_obj_t* label = lv_obj_get_child(btn, 0);

    // Cancel existing animations or timers
    lv_anim_del(btn, nullptr);
    if (label) lv_anim_del(label, nullptr);

    // Store pressed state changes inside an async callback to avoid rendering conflict
    struct TagPressContext {
        lv_obj_t* btn;
        lv_obj_t* label;
        bool pressed;
        uint32_t duration;
    };

    TagPressContext* ctx = new TagPressContext{btn, label, pressed, duration_ms};

    lv_async_call([](void* d) {
        TagPressContext* c = static_cast<TagPressContext*>(d);
        lv_obj_t* btn = c->btn;
        lv_obj_t* label = c->label;

        if (c->pressed) {
            // Apply "pressed" visual style
            lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(btn, CYBER_COLOR_ACCENT, 0); 
            lv_obj_set_style_border_width(btn, 2, 0);
            if (label) {
                lv_obj_set_style_text_color(label, CYBER_COLOR_BG, 0);
            }

            // Auto-release timer
            uint32_t delay = c->duration > 0 ? c->duration : 50;
            lv_timer_t* releaseTimer = lv_timer_create([](lv_timer_t* t) {
                lv_obj_t* btn = static_cast<lv_obj_t*>(lv_timer_get_user_data(t));
                if (!btn) return;

                lv_obj_t* label = lv_obj_get_child(btn, 0);
                lv_obj_set_style_bg_color(btn, CYBER_COLOR_BG, 0);
                lv_obj_set_style_border_color(btn, CYBER_COLOR_ACCENT, 0);
                lv_obj_set_style_border_width(btn, 1, 0);
                if (label) {
                    lv_obj_set_style_text_color(label, CYBER_COLOR_ACCENT, 0);
                }

                lv_timer_delete(t);
            }, delay, btn);
            lv_timer_set_repeat_count(releaseTimer, 1);
        } else {
            // Apply "released" visual style immediately
            lv_obj_set_style_bg_color(btn, CYBER_COLOR_BG, 0);
            lv_obj_set_style_border_color(btn, CYBER_COLOR_ACCENT, 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            if (label) {
                lv_obj_set_style_text_color(label, CYBER_COLOR_ACCENT, 0);
            }
        }

        delete c;
    }, ctx);
}

void CyberpunkLayout::setEncoderTagPressed(bool pressed, uint32_t duration_ms) {
    if (!encoderTagContainer) return;

    struct EncoderPressContext {
        lv_obj_t* container;
        bool pressed;
        uint32_t duration;
    };

    auto* ctx = new EncoderPressContext{encoderTagContainer, pressed, duration_ms};

    lv_async_call([](void* d) {
        auto* c = static_cast<EncoderPressContext*>(d);
        lv_obj_t* box = c->container;
        lv_obj_t* label = lv_obj_get_child(box, 0); // Get the rotated label

        if (!box || !label) {
            delete c;
            return;
        }

        if (c->pressed) {
            // Inverted style
            lv_obj_set_style_bg_color(box, CYBER_COLOR_ACCENT, 0);
            lv_obj_set_style_border_color(box, CYBER_COLOR_BG, 0);
            lv_obj_set_style_border_width(box, 2, 0);
            lv_obj_set_style_text_color(label, CYBER_COLOR_BG, 0);

            // Auto-release
            uint32_t delay = c->duration > 0 ? c->duration : 50;
            lv_timer_t* t = lv_timer_create([](lv_timer_t* t) {
                auto* box = static_cast<lv_obj_t*>(lv_timer_get_user_data(t));
                if (!box) return;
                lv_obj_t* label = lv_obj_get_child(box, 0);
                lv_obj_set_style_bg_color(box, CYBER_COLOR_BG, 0);
                lv_obj_set_style_border_color(box, CYBER_COLOR_ACCENT, 0);
                lv_obj_set_style_border_width(box, 1, 0);
                lv_obj_set_style_text_color(label, CYBER_COLOR_ACCENT, 0);
                lv_timer_delete(t);
            }, delay, box);
            lv_timer_set_repeat_count(t, 1);
        } else {
            lv_obj_set_style_bg_color(box, CYBER_COLOR_BG, 0);
            lv_obj_set_style_border_color(box, CYBER_COLOR_ACCENT, 0);
            lv_obj_set_style_border_width(box, 1, 0);
            lv_obj_set_style_text_color(label, CYBER_COLOR_ACCENT, 0);
        }

        delete c;
    }, ctx);
}

lv_obj_t* CyberpunkLayout::getContentArea() {
    return contentArea;
}

void CyberpunkLayout::animateIn() {
    setBottomTagsVisible(true);
    setEncoderTagVisible(true);
}

void CyberpunkLayout::animateOut() {
    setBottomTagsVisible(false);
    setEncoderTagVisible(false);
}

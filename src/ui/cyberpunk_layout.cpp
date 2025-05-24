#include "cyberpunk_layout.h"
#include "ui/cyberpunk_theme.h"
#include "constants.h"

// Animation durations and delays
constexpr uint32_t ANIM_DURATION = 300;
constexpr uint32_t ANIM_DELAY = 0;
constexpr uint32_t TAG_PRESS_DURATION = 200;

// Structure to hold auto-release animation data
struct AutoReleaseData {
    CyberpunkLayout* layout;
    size_t index;
    bool wasPressed;
};

// Animation callback for auto-release
void CyberpunkLayout::autoReleaseAnimCb(lv_anim_t* a) {
    AutoReleaseData* data = (AutoReleaseData*)a->user_data;
    data->layout->setTagPressed(data->index, !data->wasPressed);
    delete data;
}

// Animation callback for encoder auto-release
void CyberpunkLayout::autoReleaseEncoderAnimCb(lv_anim_t* a) {
    CyberpunkLayout* layout = (CyberpunkLayout*)a->user_data;
    layout->setEncoderTagPressed(false);
}

CyberpunkLayout::CyberpunkLayout(lv_obj_t* parent) {
    container = lv_obj_create(parent);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

    // Content and encoder split
    lv_obj_t* mainArea = lv_obj_create(container);
    lv_obj_set_flex_grow(mainArea, 1);
    lv_obj_set_layout(mainArea, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(mainArea, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(mainArea, 0, 0);
    lv_obj_clear_flag(mainArea, LV_OBJ_FLAG_SCROLLABLE);

    contentArea = lv_obj_create(mainArea);
    lv_obj_set_size(contentArea, DISPLAY_WIDTH - 40, DISPLAY_HEIGHT - 50);
    lv_obj_set_style_bg_color(contentArea, CYBER_COLOR_BG, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);

    // Bottom bar with equal spacing for tags
    bottomBar = lv_obj_create(mainArea);
    lv_obj_set_height(bottomBar, 40);
    lv_obj_set_flex_flow(bottomBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottomBar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(bottomBar, CYBER_COLOR_BG, 0);
    lv_obj_clear_flag(bottomBar, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create three equal-width containers for tags
    for(int i = 0; i < 3; i++) {
        lv_obj_t* tagContainer = lv_obj_create(bottomBar);
        lv_obj_set_flex_grow(tagContainer, 1);
        lv_obj_set_style_bg_opa(tagContainer, 0, 0);
        lv_obj_set_style_border_width(tagContainer, 0, 0);
        lv_obj_set_style_pad_all(tagContainer, 0, 0);
        tagContainers.push_back(tagContainer);
    }

    // Encoder tag (vertical)
    encoderTag = lv_label_create(container);
    lv_label_set_text(encoderTag, "MENU");
    lv_obj_set_style_text_color(encoderTag, CYBER_COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(encoderTag, &lv_font_montserrat_14, 0);
    lv_obj_set_style_transform_rotation(encoderTag, 900, 0); // rotate 90 degrees (900 = 90.0 degrees)
    lv_obj_align(encoderTag, LV_ALIGN_RIGHT_MID, -5, 0);
    
    // Initialize all tags as hidden
    setBottomTagsVisible(false);
    setEncoderTagVisible(false);
}

CyberpunkLayout::~CyberpunkLayout() {
    if (container) lv_obj_del(container);
}

void CyberpunkLayout::setBottomTags(const std::vector<std::string>& tags) {
    // Clear existing tags
    for(auto& label : tagLabels) {
        if(label) lv_obj_del(label);
    }
    tagLabels.clear();
    
    // Create new tags (up to 3)
    for(size_t i = 0; i < std::min(tags.size(), size_t(3)); i++) {
        lv_obj_t* label = lv_obj_create(tagContainers[i]);
        lv_label_set_text(label, tags[i].c_str());
        lv_obj_set_style_text_color(label, CYBER_COLOR_ACCENT, 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_center(label);
        tagLabels.push_back(label);
    }
    
    // Hide any unused containers
    for(size_t i = tags.size(); i < 3; i++) {
        lv_obj_add_flag(tagContainers[i], LV_OBJ_FLAG_HIDDEN);
    }
}

void CyberpunkLayout::setBottomTagsVisible(bool visible) {
    for(auto& container : tagContainers) {
        if(visible) {
            lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
            // Animate in
            lv_obj_set_style_translate_y(container, 20, 0);
            lv_obj_set_style_opa(container, 0, 0);
            
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, container);
            lv_anim_set_values(&a, 20, 0);
            lv_anim_set_time(&a, ANIM_DURATION);
            lv_anim_set_delay(&a, ANIM_DELAY);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_translate_y);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_start(&a);
            
            lv_anim_t b;
            lv_anim_init(&b);
            lv_anim_set_var(&b, container);
            lv_anim_set_values(&b, 0, LV_OPA_COVER);
            lv_anim_set_time(&b, ANIM_DURATION);
            lv_anim_set_delay(&b, ANIM_DELAY);
            lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
            lv_anim_set_path_cb(&b, lv_anim_path_ease_out);
            lv_anim_start(&b);
        } else {
            // Animate out
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, container);
            lv_anim_set_values(&a, 0, 20);
            lv_anim_set_time(&a, ANIM_DURATION);
            lv_anim_set_delay(&a, ANIM_DELAY);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_translate_y);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
            lv_anim_set_ready_cb(&a, [](lv_anim_t* a) {
                lv_obj_add_flag((lv_obj_t*)a->var, LV_OBJ_FLAG_HIDDEN);
            });
            lv_anim_start(&a);
            
            lv_anim_t b;
            lv_anim_init(&b);
            lv_anim_set_var(&b, container);
            lv_anim_set_values(&b, LV_OPA_COVER, 0);
            lv_anim_set_time(&b, ANIM_DURATION);
            lv_anim_set_delay(&b, ANIM_DELAY);
            lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
            lv_anim_set_path_cb(&b, lv_anim_path_ease_in);
            lv_anim_start(&b);
        }
    }
}

void CyberpunkLayout::setEncoderTag(const std::string& tag, bool showScrollIcon) {
    std::string display = tag;
    if (showScrollIcon) display += " ↑↓";
    lv_label_set_text(encoderTag, display.c_str());
}

void CyberpunkLayout::setEncoderTagVisible(bool visible) {
    if(visible) {
        lv_obj_clear_flag(encoderTag, LV_OBJ_FLAG_HIDDEN);
        // Animate in from right
        lv_obj_set_style_translate_x(encoderTag, 20, 0);
        lv_obj_set_style_opa(encoderTag, 0, 0);
        
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, encoderTag);
        lv_anim_set_values(&a, 20, 0);
        lv_anim_set_time(&a, ANIM_DURATION);
        lv_anim_set_delay(&a, ANIM_DELAY);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_translate_x);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
        
        lv_anim_t b;
        lv_anim_init(&b);
        lv_anim_set_var(&b, encoderTag);
        lv_anim_set_values(&b, 0, LV_OPA_COVER);
        lv_anim_set_time(&b, ANIM_DURATION);
        lv_anim_set_delay(&b, ANIM_DELAY);
        lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
        lv_anim_set_path_cb(&b, lv_anim_path_ease_out);
        lv_anim_start(&b);
    } else {
        // Animate out to right
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, encoderTag);
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
        lv_anim_set_var(&b, encoderTag);
        lv_anim_set_values(&b, LV_OPA_COVER, 0);
        lv_anim_set_time(&b, ANIM_DURATION);
        lv_anim_set_delay(&b, ANIM_DELAY);
        lv_anim_set_exec_cb(&b, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
        lv_anim_set_path_cb(&b, lv_anim_path_ease_in);
        lv_anim_start(&b);
    }
}

void CyberpunkLayout::setTagPressed(size_t index, bool pressed, uint32_t duration_ms) {
    if(index >= tagLabels.size()) return;
    
    lv_obj_t* label = tagLabels[index];
    if(!label) return;
    
    if(pressed) {
        // Visual feedback for press
        lv_obj_set_style_text_color(label, lv_color_hex(0x00FF00), 0); // Use a bright green for highlight
        lv_obj_set_style_transform_scale(label, 110, 0); // Slightly larger when pressed
        
        // If duration is specified, set up auto-release
        if(duration_ms > 0) {
            AutoReleaseData* data = new AutoReleaseData{this, index, pressed};
            
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, label);
            lv_anim_set_time(&a, duration_ms);
            lv_anim_set_user_data(&a, data);
            lv_anim_set_ready_cb(&a, autoReleaseAnimCb);
            lv_anim_start(&a);
        } else {
            // Animate back to normal state immediately
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, label);
            lv_anim_set_values(&a, 110, 100);
            lv_anim_set_time(&a, TAG_PRESS_DURATION);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_scale);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_ready_cb(&a, [](lv_anim_t* a) {
                lv_obj_set_style_text_color((lv_obj_t*)a->var, lv_color_hex(0x00FFFF), 0); // Use cyan for accent
            });
            lv_anim_start(&a);
        }
    } else {
        lv_obj_set_style_text_color(label, lv_color_hex(0x00FFFF), 0); // Use cyan for accent
        lv_obj_set_style_transform_scale(label, 100, 0);
    }
}

void CyberpunkLayout::setEncoderTagPressed(bool pressed, uint32_t duration_ms) {
    if(!encoderTag) return;
    
    if(pressed) {
        // Visual feedback for press
        lv_obj_set_style_text_color(encoderTag, lv_color_hex(0x00FF00), 0); // Use a bright green for highlight
        lv_obj_set_style_transform_scale(encoderTag, 110, 0); // Slightly larger when pressed
        
        // If duration is specified, set up auto-release
        if(duration_ms > 0) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, encoderTag);
            lv_anim_set_time(&a, duration_ms);
            lv_anim_set_user_data(&a, this);
            lv_anim_set_ready_cb(&a, autoReleaseEncoderAnimCb);
            lv_anim_start(&a);
        } else {
            // Animate back to normal state immediately
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, encoderTag);
            lv_anim_set_values(&a, 110, 100);
            lv_anim_set_time(&a, TAG_PRESS_DURATION);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_scale);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_ready_cb(&a, [](lv_anim_t* a) {
                lv_obj_set_style_text_color((lv_obj_t*)a->var, lv_color_hex(0x00FFFF), 0); // Use cyan for accent
            });
            lv_anim_start(&a);
        }
    } else {
        lv_obj_set_style_text_color(encoderTag, lv_color_hex(0x00FFFF), 0); // Use cyan for accent
        lv_obj_set_style_transform_scale(encoderTag, 100, 0);
    }
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

#include "main_menu_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include "ui/cyberpunk_theme.h"

MainMenuController& MainMenuController::getInstance() {
    static MainMenuController instance;
    return instance;
}

void MainMenuController::buildView(lv_obj_t* parent) {
    printf("Building Main Menu View\n");

    CyberpunkTheme::init();
    buttons.clear();
    printf("Cleared buttons\n");

    // Root container (fills screen, vertical layout)
    lv_obj_t* root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_margin_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);

    // Title bar (fixed, not scrollable)
    lv_obj_t* title = CyberpunkTheme::createStripedTitleLabel(root, "REFLOW OVEN");
    lv_obj_clear_flag(title, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_margin_bottom(title, 0, 0);
    lv_obj_set_style_pad_bottom(title, 0, 0);

    // Scrollable button list container
    menu = lv_obj_create(root);
    lv_obj_remove_style_all(menu);
    lv_obj_set_width(menu, DISPLAY_WIDTH);
    lv_obj_set_flex_grow(menu, 1);
    lv_obj_set_scroll_dir(menu, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(menu, LV_SCROLL_SNAP_START);
    lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_layout(menu, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu, 15, 0);
    lv_obj_set_style_pad_bottom(menu, 20, 0);  // Bottom breathing room
    lv_obj_set_style_pad_top(menu, 0, 0);
    lv_obj_set_style_border_width(menu, 0, 0);
    lv_obj_set_style_bg_color(menu, CYBER_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(menu, LV_OPA_COVER, 0);
    lv_obj_set_style_margin_top(menu, 0, 0);

    // Menu items
    const char* items[] = {
        "Start Reflow",
        "Select Profile",
        "Calibration",
        "Settings",
        "Open/Close Door"
    };

    for (int i = 0; i < 5; ++i) {
        lv_obj_t* btn = lv_btn_create(menu);
        lv_obj_set_size(btn, 280, 45);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS); // Focus will scroll into view

        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, items[i]);
        lv_obj_center(label);

        if (i == selectedIndex) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x404040), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
        }

        buttons.push_back(btn);
    }

    // Ensure the menu is scrolled to the top when first loaded
    updateButtonFocus(false);
}


void MainMenuController::updateButtonFocus(bool animated) {
    if (!menu || buttons.empty()) return;

    // Safely check valid index range
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(buttons.size())) {
        selectedIndex = 0; // Reset to safe value
    }

    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        lv_obj_t* btn = buttons[i];
        if (!lv_obj_is_valid(btn)) continue;

        if (i == selectedIndex) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            if (animated) {
                // Configure scrolling animation properties for menu
                lv_obj_set_scroll_snap_y(menu, LV_SCROLL_SNAP_START);
                
                // Set animation duration for the scroll container
                lv_obj_set_style_anim_time(menu, 300, 0);  // 300ms animation duration
                
                // Use a safer version that checks if object is still valid
                if (lv_obj_is_valid(menu) && lv_obj_is_valid(btn)) {
                    lv_obj_scroll_to_view(btn, LV_ANIM_ON);
                }
            } else {
                if (lv_obj_is_valid(menu) && lv_obj_is_valid(btn)) {
                    lv_obj_scroll_to_view(btn, LV_ANIM_OFF);
                }
            }
        } else {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x404040), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
        }
    }
}


void MainMenuController::init() {}

void MainMenuController::onEncoderPress() {
    BuzzerService::getInstance().playMediumTone(300);

    switch (selectedIndex) {
        case 0: startReflow(); break;
        case 1: selectReflowCurve(); break;
        case 2: calibrate(); break;
        case 3: openSettings(); break;
        case 4: toggleDoor(); break;
    }
}

void MainMenuController::onEncoderUp() {
    // Prevent UI updates if objects aren't valid
    if (!menu || buttons.empty()) return;
    
    if (selectedIndex > 0) {
        selectedIndex--;
        updateButtonFocus();
    }
}

void MainMenuController::onEncoderDown() {
    // Prevent UI updates if objects aren't valid
    if (!menu || buttons.empty()) return;
    
    if (selectedIndex < static_cast<int>(buttons.size()) - 1) {
        selectedIndex++;
        updateButtonFocus();
    }
}

void MainMenuController::onEncoderLongPress() {
    // No back action in main menu
}

void MainMenuController::selectReflowCurve() {
    // navigateTo("profile-selection", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::startReflow() {
    // navigateTo("reflow", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrate() {
    // navigateTo("calibration", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::openSettings() {
    // navigateTo("settings", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::toggleDoor() {
    if (DoorService::getInstance().isFullyOpen()) {
        DoorService::getInstance().setPosition(0);
    } else {
        DoorService::getInstance().setPosition(100);
    }
}

void MainMenuController::willUnload() {
    if (menu) {
        lv_obj_del(menu);
        menu = nullptr;
    }
    buttons.clear();
    selectedIndex = 0;
}

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
    // Remove all gaps between flex items
    lv_obj_set_style_pad_row(root, 0, 0);
    lv_obj_set_style_pad_column(root, 0, 0);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_margin_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);

    // Title bar (fixed, not scrollable)
    lv_obj_t* title = CyberpunkTheme::createStripedTitleLabel(root, "REFLOW OVEN", DISPLAY_WIDTH, 20);
    lv_obj_clear_flag(title, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_margin_all(title, 0, 0);
    lv_obj_set_style_pad_all(title, 0, 0);

    // Scrollable button list container
    menu = lv_obj_create(root);
    lv_obj_remove_style_all(menu);
    lv_obj_set_width(menu, DISPLAY_WIDTH);
    lv_obj_set_flex_grow(menu, 1);
    lv_obj_set_style_margin_all(menu, 0, 0);
    lv_obj_set_style_pad_top(menu, 10, 0);
    lv_obj_set_scroll_dir(menu, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(menu, LV_SCROLL_SNAP_START);
    lv_obj_set_scrollbar_mode(menu, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_layout(menu, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu, 15, 0);
    lv_obj_set_style_pad_bottom(menu, 20, 0);  // Bottom breathing room
    lv_obj_set_style_border_width(menu, 0, 0);
    lv_obj_set_style_bg_color(menu, CYBER_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(menu, LV_OPA_COVER, 0);

    // Force creating the first button at the top
    lv_obj_t* topPadding = lv_obj_create(menu);
    lv_obj_set_height(topPadding, 1); // Minimal height
    lv_obj_set_width(topPadding, DISPLAY_WIDTH);
    lv_obj_remove_style_all(topPadding);
    lv_obj_set_style_bg_opa(topPadding, LV_OPA_TRANSP, 0);

    // Menu items
    const char* items[] = {
        "Start Reflow",
        "Select Profile",
        "Calibration",
        "Settings",
        "Open/Close Door"
    };

    for (int i = 0; i < 5; ++i) {
        char indexStr[6]; // Buffer for "XX/XX" format
        snprintf(indexStr, sizeof(indexStr), "%02d/%02d", i+1, 5); // Format as "01/05", "02/05", etc.
        lv_obj_t* btn = CyberpunkTheme::createCyberpunkButton(menu, items[i], indexStr, i == selectedIndex);
        buttons.push_back(btn);
    }

    // Ensure the menu is scrolled to the top when first loaded
    updateButtonFocus(false);
}


void MainMenuController::updateButtonFocus(bool animated) {
    if (!menu || buttons.empty()) return;

    // Clamp selection
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(buttons.size())) {
        selectedIndex = 0;
    }

    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        lv_obj_t* btn = buttons[i];
        if (!lv_obj_is_valid(btn)) continue;

        bool isSelected = (i == selectedIndex);

        lv_color_t bgColor     = isSelected ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG;
        lv_color_t textColor   = isSelected ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;
        lv_color_t borderColor = isSelected ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;

        lv_obj_set_style_bg_color(btn, bgColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, borderColor, LV_PART_MAIN);

        // Update child labels (assumes 2 children)
        uint32_t childCount = lv_obj_get_child_cnt(btn);
        for (uint32_t j = 0; j < childCount; ++j) {
            lv_obj_t* child = lv_obj_get_child(btn, j);
            if (!lv_obj_is_valid(child)) continue;

            // Apply style to both labels
            lv_obj_set_style_text_color(child, textColor, LV_PART_MAIN);
        }

        if (isSelected) {
            if (animated) {
                lv_obj_set_scroll_snap_y(menu, LV_SCROLL_SNAP_START);
                lv_obj_set_style_anim_time(menu, 300, 0);
                lv_obj_scroll_to_view(btn, LV_ANIM_ON);
            } else {
                lv_obj_scroll_to_view(btn, LV_ANIM_OFF);
            }
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
    printf("Selecting reflow curve\n");
    // navigateTo("profile-selection", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::startReflow() {
    printf("Starting reflow\n");
    // navigateTo("reflow", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrate() {
    printf("Calibrating\n");
    // navigateTo("calibration", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::openSettings() {
    printf("Opening settings\n");
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

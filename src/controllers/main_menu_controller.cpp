#include "main_menu_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"

MainMenuController& MainMenuController::getInstance() {
    static MainMenuController instance;
    return instance;
}

void MainMenuController::buildView(lv_obj_t* parent) {
    printf("Building Main Menu View\n");
    
    buttons.clear();
    printf("Cleared buttons\n");

    // Create scrollable menu inside parent
    menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_set_style_bg_color(menu, lv_color_hex(0x202020), LV_PART_MAIN);

    // Set flex layout and enable vertical scrolling
    lv_obj_set_layout(menu, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(menu, LV_DIR_VER);  // Enable vertical scrolling
    lv_obj_clear_flag(menu, LV_OBJ_FLAG_SCROLLABLE); // Optional: remove if you want default scrollbars
    lv_obj_set_scroll_snap_y(menu, LV_SCROLL_SNAP_CENTER); // Optional: snap to buttons
    lv_obj_set_style_pad_row(menu, 10, 0);
    lv_obj_set_style_pad_all(menu, 10, 0);

    // Title label
    lv_obj_t* title = lv_label_create(menu);
    lv_label_set_text(title, "Reflow Oven");
    lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_bottom(title, 20, 0);

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
        lv_obj_set_size(btn, 200, 40);
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
}


void MainMenuController::updateButtonFocus() {
    if (!menu || buttons.empty()) return;

    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        lv_obj_t* btn = buttons[i];
        if (!lv_obj_is_valid(btn)) continue;

        if (i == selectedIndex) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
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
    if (selectedIndex > 0) {
        selectedIndex--;
        updateButtonFocus();
    }
}

void MainMenuController::onEncoderDown() {
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

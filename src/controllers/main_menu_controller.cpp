#include "main_menu_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"

MainMenuController& MainMenuController::getInstance() {
    static MainMenuController instance;
    return instance;
}

void MainMenuController::render() {
    Controller::render(); // Call parent render to initialize the screen
    
    if (!menu) {
        // Create menu container
        menu = lv_obj_create(screen);
        lv_obj_set_size(menu, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(menu, lv_color_hex(0x202020), LV_PART_MAIN);
        lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(menu, 10, 0);
        
        // Title
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
        
        // Create buttons for each menu item
        for (int i = 0; i < 5; i++) {
            lv_obj_t* btn = lv_btn_create(menu);
            lv_obj_set_size(btn, 200, 40);
            
            // Set button style directly
            if (i == selectedIndex) {
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
                lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            } else {
                lv_obj_set_style_bg_color(btn, lv_color_hex(0x404040), LV_PART_MAIN);
                lv_obj_set_style_text_color(btn, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
            }
            
            lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
            
            lv_obj_t* label = lv_label_create(btn);
            lv_label_set_text(label, items[i]);
            lv_obj_center(label);
        }
    }
    
    // Update button focus based on selectedIndex
    updateButtonFocus();
}

void MainMenuController::init() {
    // Initialize controller resources
}

void MainMenuController::onEncoderPress() {
    // Handle button press based on selectedIndex
    BuzzerService::getInstance().playMediumTone(300);
    
    switch (selectedIndex) {
        case 0: // Start Reflow
            startReflow();
            break;
        case 1: // Select Profile
            selectReflowCurve();
            break;
        case 2: // Calibration
            calibrate();
            break;
        case 3: // Settings
            openSettings();
            break;
        case 4: // Toggle Door
            toggleDoor();
            break;
    }
}

void MainMenuController::onEncoderUp() {
    // Navigate up in the menu
    if (selectedIndex > 0) {
        selectedIndex--;
        updateButtonFocus();
    }
}

void MainMenuController::onEncoderDown() {
    // Navigate down in the menu
    if (selectedIndex < 4) { // 5 items total (0-4)
        selectedIndex++;
        updateButtonFocus();
    }
}

void MainMenuController::onEncoderLongPress() {
    // No action for long press in main menu
}

void MainMenuController::selectReflowCurve() {
    navigateTo("profile-selection", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::startReflow() {
    navigateTo("reflow", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrate() {
    navigateTo("calibration", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::openSettings() {
    navigateTo("settings", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::toggleDoor() {
    if (DoorService::getInstance().isFullyOpen()) {
        DoorService::getInstance().setPosition(0); // Close door
    } else {
        DoorService::getInstance().setPosition(100); // Open door
    }
}

// Helper to update button focus
void MainMenuController::updateButtonFocus() {
    if (!menu) return;
    
    int i = 0;
    for (lv_obj_t* btn = lv_obj_get_child(menu, 1); btn != NULL; btn = lv_obj_get_child(menu, i + 2), i++) {
        // Update button style directly
        if (i == selectedIndex) {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x404040), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
        }
    }
}

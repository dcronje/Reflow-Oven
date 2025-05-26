#include "main_menu_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include "core/event_bus.h"
#include "core/event_payload.h"
#include "types/event_types.h"
#include "ui/cyberpunk_theme.h"

MainMenuController& MainMenuController::getInstance() {
    static MainMenuController instance;
    return instance;
}

void MainMenuController::buildView(lv_obj_t* parent) {
    if (!parent || !lv_obj_is_valid(parent)) {
        printf("ERROR: Invalid parent in buildView\n");
        return;
    }

    buttons.clear();

    // Root container (fills screen, vertical layout)
    lv_obj_t* root = lv_obj_create(parent);
    if (!root) {
        printf("ERROR: Failed to create root container\n");
        return;
    }
    
    // Verify root container is properly attached to parent
    lv_obj_t* rootParent = lv_obj_get_parent(root);
    if (rootParent != parent) {
        printf("ERROR: Root container parent mismatch - container: %p, parent: %p\n", (void*)rootParent, (void*)parent);
        return;
    }
    
    lv_obj_set_size(root, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(root, 0, 0);
    lv_obj_set_style_pad_column(root, 0, 0);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_margin_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);

    // Title bar (fixed, not scrollable)
    lv_obj_t* title = CyberpunkTheme::createStripedTitleLabel(
        root, 
        "MAIN MENU", 
        DISPLAY_WIDTH, 
        40,
        24,                        // Stripe thickness
        lv_color_white(),          // Text color
        lv_color_hex(0x000000),    // Outline color
        2                          // Outline thickness
    );
    if (!title) {
        printf("ERROR: Failed to create title bar\n");
        return;
    }
    
    lv_obj_clear_flag(title, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_margin_all(title, 0, 0);
    lv_obj_set_style_pad_all(title, 0, 0);

    // Scrollable button list container
    menu = lv_obj_create(root);
    if (!menu) {
        printf("ERROR: Failed to create menu container\n");
        return;
    }
    
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
        "SETTINGS",
        "VIEW CURVES",
        "CALIBRATE SENSORS",
        "CALIBRATE DOOR",
        "CALIBRATE OVEN",
        "HOME"
    };

    for (int i = 0; i < 6; ++i) {
        char indexStr[6]; // Buffer for "XX/XX" format
        snprintf(indexStr, sizeof(indexStr), "%02d/%02d", i+1, 6);
        
        lv_obj_t* btn = CyberpunkTheme::createCyberpunkButton(menu, items[i], indexStr, i == selectedIndex);
        if (!btn) {
            printf("ERROR: Failed to create button %d\n", i);
            continue;
        }
        
        if (!lv_obj_is_valid(btn)) {
            printf("ERROR: Button %d is invalid\n", i);
            continue;
        }
        
        buttons.push_back(btn);
    }

    // Ensure the menu is scrolled to the top when first loaded
    updateButtonFocus(false);
}

void MainMenuController::updateButtonFocus(bool animated) {
    if (!menu || !lv_obj_is_valid(menu)) {
        printf("ERROR: Invalid menu in updateButtonFocus\n");
        return;
    }
    
    if (buttons.empty()) {
        printf("ERROR: No buttons in updateButtonFocus\n");
        return;
    }

    // Clamp selection
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(buttons.size())) {
        selectedIndex = 0;
    }

    // Verify all buttons are valid before proceeding
    for (size_t i = 0; i < buttons.size(); ++i) {
        lv_obj_t* btn = buttons[i];
        if (!btn || !lv_obj_is_valid(btn)) {
            printf("ERROR: Button %zu is invalid, rebuilding view\n", i);
            if (rootView) {
                buildView(rootView);
            }
            return;
        }
    }

    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        lv_obj_t* btn = buttons[i];
        
        // Double check validity since we're in a loop
        if (!btn || !lv_obj_is_valid(btn)) {
            printf("ERROR: Button %d became invalid during update!\n", i);
            continue;
        }

        bool isSelected = (i == selectedIndex);

        // Update button styles
        lv_color_t bgColor     = isSelected ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG;
        lv_color_t textColor   = isSelected ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;
        lv_color_t borderColor = isSelected ? CYBER_COLOR_BG : CYBER_COLOR_ACCENT;

        lv_obj_set_style_bg_color(btn, bgColor, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, borderColor, LV_PART_MAIN);

        // Update child labels (assumes 2 children)
        uint32_t childCount = lv_obj_get_child_cnt(btn);
        
        // Verify we have the expected number of children
        if (childCount != 2) {
            printf("WARNING: Button %d has unexpected number of children (%u)\n", i, childCount);
            continue;
        }
        
        for (uint32_t j = 0; j < childCount; ++j) {
            lv_obj_t* child = lv_obj_get_child(btn, j);
            
            if (!child || !lv_obj_is_valid(child)) {
                printf("ERROR: Button %d child %u is invalid!\n", i, j);
                continue;
            }

            // Verify child is a label before setting text color
            if (lv_obj_has_class(child, &lv_label_class)) {
                lv_obj_set_style_text_color(child, textColor, LV_PART_MAIN);
            } else {
                printf("WARNING: Button %d child %u is not a label!\n", i, j);
            }
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

MainMenuController::~MainMenuController() {
    // Clean up resources
    if (updateTimer) {
        lv_timer_del(updateTimer);
        updateTimer = nullptr;
    }

    // Delete the event processing task if it exists
    if (eventTaskHandle != nullptr) {
        vTaskDelete(eventTaskHandle);
        eventTaskHandle = nullptr;
    }
}

void MainMenuController::init() {
    // Create a task to process events from the EventBus
    xTaskCreate(
        eventProcessingTask,   // Function that implements the task
        "MenuEventTask",       // Task name
        256,                   // Stack size in words
        this,                  // Parameter passed to the task
        1,                     // Task priority
        &eventTaskHandle       // Task handle
    );
    
    // Create timer for periodic UI updates (500ms)
    updateTimer = lv_timer_create(updateTimerCallback, 500, this);
}

void MainMenuController::eventProcessingTask(void* pvParameters) {
    MainMenuController* controller = static_cast<MainMenuController*>(pvParameters);
    
    while (true) {
        // Wait for and process events
        Event event(EventTopics::SYSTEM, EventNames::INITIALIZED);
        if (controller->eventSubscriber.receiveEvent(event, pdMS_TO_TICKS(100))) {
            // Handle events based on topic and event name
            if (event.topic == EventTopics::DOOR) {
                if (event.event == EventNames::OPENED || 
                    event.event == EventNames::CLOSED ||
                    event.event == EventNames::OPENING || 
                    event.event == EventNames::CLOSING) {
                    // Door state changed - we'll let the timer update the UI
                    // to avoid threading issues with direct UI updates
                }
            }
            
            // Clean up event - the Event destructor will handle freeing the payload
        }
        
        // Brief delay if no events
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void MainMenuController::updateTimerCallback(lv_timer_t* timer) {
    // Cast the user_data directly without referencing timer->user_data
    // to avoid the linter error with incomplete type
    void* userData = lv_timer_get_user_data(timer);
    MainMenuController* controller = static_cast<MainMenuController*>(userData);
    if (controller) {
        controller->periodicUpdate();
    }
}

void MainMenuController::periodicUpdate() {
    // No periodic updates needed anymore
}

void MainMenuController::onEncoderPress() {
    BuzzerService::getInstance().playMediumTone(300);

    switch (selectedIndex) {
        case 0: openSettings(); break;
        case 1: viewCurves(); break;
        case 2: calibrateSensors(); break;
        case 3: calibrateDoor(); break;
        case 4: calibrateOven(); break;
        case 5: returnToHome(); break;
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

void MainMenuController::viewCurves() {
    printf("Viewing curves\n");
    // navigateTo("curves", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrateSensors() {
    printf("Calibrating sensors\n");
    // navigateTo("sensor-calibration", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrateDoor() {
    printf("Calibrating door\n");
    // navigateTo("door-calibration", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrateOven() {
    printf("Calibrating oven\n");
    // navigateTo("oven-calibration", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::openSettings() {
    printf("Opening settings\n");
    // navigateTo("settings", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::returnToHome() {
    printf("MainMenuController::returnToHome\n");
    navigateToSafe("home", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::viewDidLoad() {
    printf("MainMenuController::viewDidLoad\n");
}

void MainMenuController::viewWillAppear() {
    printf("MainMenuController::viewWillAppear\n");
    startEventTask();
    startUpdateTimer();
}

void MainMenuController::viewDidAppear() {
    printf("MainMenuController::viewDidAppear\n");
}

void MainMenuController::viewWillDisappear() {
    printf("MainMenuController::viewWillDisappear\n");
    stopEventTask();
    stopUpdateTimer();
}

void MainMenuController::viewDidDisappear() {
    printf("MainMenuController::viewDidDisappear\n");
}

void MainMenuController::viewWillUnload() {
    printf("MainMenuController::viewWillUnload\n");
    stopEventTask();
    stopUpdateTimer();
    
    if (menu) {
        lv_obj_del(menu);
        menu = nullptr;
    }
}

void MainMenuController::startEventTask() {
    if (eventTaskHandle == nullptr) {
        xTaskCreate(
            eventProcessingTask,
            "MenuEventTask",
            256,
            this,
            1,
            &eventTaskHandle
        );
    }
}

void MainMenuController::stopEventTask() {
    if (eventTaskHandle != nullptr) {
        vTaskDelete(eventTaskHandle);
        eventTaskHandle = nullptr;
    }
}

void MainMenuController::startUpdateTimer() {
    if (updateTimer == nullptr) {
        updateTimer = lv_timer_create(updateTimerCallback, 500, this);
    }
}

void MainMenuController::stopUpdateTimer() {
    if (updateTimer != nullptr) {
        lv_timer_pause(updateTimer);
        lv_timer_del(updateTimer);
        updateTimer = nullptr;
    }
}

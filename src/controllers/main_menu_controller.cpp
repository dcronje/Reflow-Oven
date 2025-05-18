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
    lv_obj_t* title = CyberpunkTheme::createStripedTitleLabel(
        root, 
        "REFLOW OVEN", 
        DISPLAY_WIDTH, 
        24,                        // Stripe thickness
        lv_color_white(),          // Text color
        lv_color_hex(0x000000),    // Outline color
        2                          // Outline thickness
    );
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
        "START REFLOW",
        "SELECT PROFILE",
        "CALIBRATE",
        "SETTINGS",
        DoorService::getInstance().isFullyOpen() ? "CLOSE DOOR" : "OPEN DOOR"
    };

    for (int i = 0; i < 5; ++i) {
        char indexStr[6]; // Buffer for "XX/XX" format
        snprintf(indexStr, sizeof(indexStr), "%02d/%02d", i+1, 5); // Format as "01/05", "02/05", etc.
        
        // Use dynamic text for the door button (last item)
        const char* buttonText = (i == 4) ? 
            (DoorService::getInstance().isFullyOpen() ? "CLOSE DOOR" : "OPEN DOOR") : 
            items[i];
            
        lv_obj_t* btn = CyberpunkTheme::createCyberpunkButton(menu, buttonText, indexStr, i == selectedIndex);
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
    // This method is called periodically from the updateTimer
    // Use it to safely update UI from the LVGL context
    refreshDoorStatusButton();
}

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
    
    // Example: Post an event to the EventBus that reflow is starting
    // Create event with no payload
    Event event(EventTopics::REFLOW, EventNames::STARTED);
    
    // Post to the event bus
    EventBus::getInstance().postEvent(event);
    
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
    // Toggle door position via DoorService
    DoorService& doorService = DoorService::getInstance();
    
    if (doorService.isFullyOpen()) {
        doorService.close(); // Close door
    } else {
        doorService.open();  // Open door
    }
    
    // Door service will publish its own events about state changes
    // The controller now only issues commands and doesn't publish events
}

void MainMenuController::refreshDoorStatusButton() {
    // Make sure we have buttons and the door button exists (it's the last one)
    if (buttons.empty() || buttons.size() < 5) return;
    
    // Update the text of the door button (last button)
    lv_obj_t* doorBtn = buttons[4];
    if (!lv_obj_is_valid(doorBtn)) return;
    
    // Get the label (should be first child)
    lv_obj_t* label = lv_obj_get_child(doorBtn, 0);
    if (!lv_obj_is_valid(label)) return;
    
    // Update the text based on current door state
    const char* doorText = nullptr;
    
    // Determine the appropriate text based on door state
    DoorService& doorService = DoorService::getInstance();
    
    // Get the direction of movement
    DoorDirection direction = doorService.getDoorDirection();
    bool isMoving = (direction != DoorDirection::NONE);
    
    if (isMoving) {
        // Door is moving - show status with animation indicator
        if (direction == DoorDirection::OPENING) {
            doorText = "OPENING...";
        } else {
            doorText = "CLOSING...";
        }
        
        // Add a visual indicator for motion - flash the button
        static bool alternateColor = false;
        alternateColor = !alternateColor;
        
        // Alternate between normal and warning colors
        lv_color_t bgColor = alternateColor ? 
            CYBER_COLOR_WARNING : // Warning color during animation
            (doorBtn == buttons[selectedIndex] ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG);
            
        lv_obj_set_style_bg_color(doorBtn, bgColor, LV_PART_MAIN);
    } else {
        // Door is stationary
        if (doorService.isFullyOpen()) {
            doorText = "CLOSE DOOR";
        } else if (doorService.isFullyClosed()) {
            doorText = "OPEN DOOR";
        } else {
            // Door is stopped between open and closed
            if (doorService.getPosition() > 50) {
                doorText = "CLOSE DOOR"; // More open than closed
            } else {
                doorText = "OPEN DOOR";  // More closed than open
            }
        }
        
        // Restore normal button color based on selection state
        bool isSelected = (doorBtn == buttons[selectedIndex]);
        lv_color_t bgColor = isSelected ? CYBER_COLOR_ACCENT : CYBER_COLOR_BG;
        lv_obj_set_style_bg_color(doorBtn, bgColor, LV_PART_MAIN);
    }
    
    // Update the button text
    lv_label_set_text(label, doorText);
}

void MainMenuController::willUnload() {
    printf("MainMenuController willUnload\n");
    
    // Clean up timers
    if (updateTimer) {
        lv_timer_del(updateTimer);
        updateTimer = nullptr;
    }
    
    // Clean up the event task
    if (eventTaskHandle != nullptr) {
        vTaskDelete(eventTaskHandle);
        eventTaskHandle = nullptr;
    }
    
    // Clean up UI
    if (menu) {
        lv_obj_del(menu);
        menu = nullptr;
    }
    buttons.clear();
    selectedIndex = 0;
}

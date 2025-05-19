#include "main_menu_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include "core/message_event_bus.h"
#include "core/message_event_adapter.h"
#include "ui/cyberpunk_theme.h"
#include "pb_decode.h"
#include "pb.h"
#include "pb_encode.h"
#include <ctime>
#include <cstring>

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

    // Delete the message processing task if it exists
    if (messageTaskHandle != nullptr) {
        vTaskDelete(messageTaskHandle);
        messageTaskHandle = nullptr;
    }
    
    // Delete message queue
    if (messageQueue != nullptr) {
        vQueueDelete(messageQueue);
        messageQueue = nullptr;
    }
    
    // Unregister from message event bus
    MessageEventBus::getInstance().unregisterHandler(this);
}

void MainMenuController::init() {
    // Create message queue
    messageQueue = xQueueCreate(10, MessageEventBus::MAX_MESSAGE_SIZE);
    
    // Register with MessageEventBus
    MessageEventBus::getInstance().registerHandler(this);
    
    // Register for specific message types
    MessageEventBus::getInstance().registerTypeHandler(
        static_cast<uint32_t>(MessageType::SYSTEM), this);
    MessageEventBus::getInstance().registerTypeHandler(
        static_cast<uint32_t>(MessageType::HARDWARE), this);
    
    // Create a task to process messages
    xTaskCreate(
        messageProcessingTask,   // Function that implements the task
        "MenuMsgTask",           // Task name
        256,                     // Stack size in words
        this,                    // Parameter passed to the task
        1,                       // Task priority
        &messageTaskHandle       // Task handle
    );
    
    // Create timer for periodic UI updates (500ms)
    updateTimer = lv_timer_create(updateTimerCallback, 500, this);
}

bool MainMenuController::processMessage(const void* data, size_t size) {
    if (data == nullptr || size == 0) {
        return false;
    }
    
    // Simply forward the message to our queue for processing in the task
    return xQueueSend(messageQueue, data, 0) == pdTRUE;
}

bool MainMenuController::processMessage(const std::string& serialized) {
    if (serialized.empty()) {
        return false;
    }
    return processMessage(serialized.data(), serialized.size());
}

void MainMenuController::messageProcessingTask(void* pvParameters) {
    MainMenuController* controller = static_cast<MainMenuController*>(pvParameters);
    
    if (!controller || !controller->messageQueue) {
        vTaskDelete(nullptr); // Delete self if invalid
        return;
    }
    
    // Buffer for receiving messages
    uint8_t messageBuffer[MessageEventBus::MAX_MESSAGE_SIZE];
    
    while (true) {
        // Wait for a message
        if (xQueueReceive(controller->messageQueue, messageBuffer, portMAX_DELAY) == pdTRUE) {
            // First try to parse as a hardware message for door events
            if (controller->processDoorMessage(messageBuffer, sizeof(messageBuffer))) {
                // Successfully processed as a door message
            }
            // Then try to parse as a system message
            else if (controller->processSystemMessage(messageBuffer, sizeof(messageBuffer))) {
                // Successfully processed as a system message
            }
            // Other message types can be added here
        }
        
        // Brief delay if message queue is empty
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool MainMenuController::processDoorMessage(const void* data, size_t size) {
    if (data == nullptr || size == 0) {
        return false;
    }
    
    // Parse the message as a hardware message
    reflow_HardwareMessage message = reflow_HardwareMessage_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(static_cast<const pb_byte_t*>(data), size);
    
    if (!pb_decode(&stream, reflow_HardwareMessage_fields, &message)) {
        return false;
    }
    
    // Check if it's a door event
    if (message.which_payload == reflow_HardwareMessage_door_event_tag) {
        // Door state changed - we'll let the timer update the UI
        // to avoid threading issues with direct UI updates
        return true;
    }
    
    return false;
}

bool MainMenuController::processSystemMessage(const void* data, size_t size) {
    if (data == nullptr || size == 0) {
        return false;
    }
    
    // Parse the message as a system message
    reflow_SystemMessage message = reflow_SystemMessage_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(static_cast<const pb_byte_t*>(data), size);
    
    if (!pb_decode(&stream, reflow_SystemMessage_fields, &message)) {
        return false;
    }
    
    // Process based on message type
    if (message.which_payload == reflow_SystemMessage_event_tag) {
        // System state changed - we might want to update the UI
        // based on the new state
        return true;
    }
    
    return false;
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
    
    // Create and serialize a system command to start reflow
    reflow_SystemMessage message = reflow_SystemMessage_init_zero;
    message.has_base = true;
    message.base.sequence_id = 0;
    message.base.timestamp = time(nullptr) * 1000;
    message.base.priority = 1;
    message.base.type = reflow_BaseMessage_MessageType_COMMAND;
    
    // Set command type
    message.which_payload = reflow_SystemMessage_command_tag;
    message.payload.command.command = reflow_SystemCommand_CommandType_START_REFLOW;
    
    // Serialize and post to the message event bus
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (pb_encode(&stream, reflow_SystemMessage_fields, &message)) {
        MessageEventAdapter::getInstance().postMessage(buffer, stream.bytes_written);
    }
    
    // navigateTo("reflow", 300, TransitionDirection::SLIDE_OUT_LEFT);
}

void MainMenuController::calibrate() {
    printf("Calibrating\n");
    
    // Create and serialize a system command to start calibration
    reflow_SystemMessage message = reflow_SystemMessage_init_zero;
    message.has_base = true;
    message.base.sequence_id = 0;
    message.base.timestamp = time(nullptr) * 1000;
    message.base.priority = 1;
    message.base.type = reflow_BaseMessage_MessageType_COMMAND;
    
    // Set command type
    message.which_payload = reflow_SystemMessage_command_tag;
    message.payload.command.command = reflow_SystemCommand_CommandType_START_CALIBRATION;
    
    // Serialize and post to the message event bus
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (pb_encode(&stream, reflow_SystemMessage_fields, &message)) {
        MessageEventAdapter::getInstance().postMessage(buffer, stream.bytes_written);
    }
    
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
    
    // Unregister from message event bus
    MessageEventBus::getInstance().unregisterHandler(this);
    
    // Clean up the message task
    if (messageTaskHandle != nullptr) {
        vTaskDelete(messageTaskHandle);
        messageTaskHandle = nullptr;
    }
    
    // Delete message queue
    if (messageQueue != nullptr) {
        vQueueDelete(messageQueue);
        messageQueue = nullptr;
    }
    
    // Clean up UI
    if (menu) {
        lv_obj_del(menu);
        menu = nullptr;
    }
    buttons.clear();
    selectedIndex = 0;
}

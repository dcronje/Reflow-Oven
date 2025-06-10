#include "home_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include "ui/cyberpunk_theme.h"
#include "services/sensor_service.h"
#include "services/temperature_control_service.h"

HomeController& HomeController::getInstance() {
    static HomeController instance;
    return instance;
}

HomeController::~HomeController() {
    // Clean up resources
    if (updateTimer) {
        lv_timer_del(updateTimer);
        updateTimer = nullptr;
    }
}

void HomeController::startUpdateTimer() {
    if (updateTimer == nullptr) {
        updateTimer = lv_timer_create(updateTimerCallback, 500, this);
    }
}

void HomeController::stopUpdateTimer() {
    if (updateTimer != nullptr) {
        lv_timer_pause(updateTimer);
        lv_timer_del(updateTimer);
        updateTimer = nullptr;
    }
}

void HomeController::updateTimerCallback(lv_timer_t* timer) {
    void* userData = lv_timer_get_user_data(timer);
    HomeController* controller = static_cast<HomeController*>(userData);
    if (controller) {
        controller->periodicUpdate();
    }
}

void HomeController::periodicUpdate() {
    // Update UI elements that need periodic refresh
    updateTags();
    updateStatusDisplay();
}

void HomeController::buildView(lv_obj_t* parent) {
    if (!parent || !lv_obj_is_valid(parent)) {
        printf("ERROR: Invalid parent object in buildView\n");
        return;
    }
    
    // Initialize the theme
    CyberpunkTheme::init();
    
    // Create layout using the provided parent
    layout = new CyberpunkLayout(parent);
    if (!layout) {
        printf("ERROR: Failed to create layout\n");
        return;
    }

    // Ensure the parent is properly sized
    lv_obj_set_size(parent, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_margin_all(parent, 0, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    // Encoder tag (vertical)
    layout->setEncoderTag("MENU", false);
    layout->setEncoderTagVisible(true);
    
    updateTags();
    
    // Create status display container
    lv_obj_t* content = layout->getContentArea();
    if (!content || !lv_obj_is_valid(content)) {
        printf("ERROR: Invalid content area in buildView\n");
        return;
    }

    // Create a container for the status display
    lv_obj_t* statusContainer = lv_obj_create(content);
    if (!statusContainer) {
        printf("ERROR: Failed to create status container\n");
        return;
    }

    // Style the container
    lv_obj_set_size(statusContainer, lv_pct(90), lv_pct(80));
    lv_obj_center(statusContainer);
    lv_obj_set_style_bg_color(statusContainer, CYBER_COLOR_BG, 0);
    lv_obj_set_style_border_color(statusContainer, CYBER_COLOR_ACCENT, 0);
    lv_obj_set_style_border_width(statusContainer, 2, 0);
    lv_obj_set_style_pad_all(statusContainer, 20, 0);
    lv_obj_set_flex_flow(statusContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(statusContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(statusContainer, 10, 0);

    // Create status labels
    auto createStatusLabel = [statusContainer](const char* label, const char* value) {
        lv_obj_t* container = lv_obj_create(statusContainer);
        lv_obj_set_size(container, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
        lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Label
        lv_obj_t* labelObj = lv_label_create(container);
        lv_label_set_text(labelObj, label);
        lv_obj_set_style_text_color(labelObj, CYBER_COLOR_ACCENT, 0);
        lv_obj_set_style_text_font(labelObj, &lv_font_montserrat_16, 0);

        // Value
        lv_obj_t* valueObj = lv_label_create(container);
        lv_label_set_text(valueObj, value);
        lv_obj_set_style_text_color(valueObj, CYBER_COLOR_TEXT, 0);
        lv_obj_set_style_text_font(valueObj, &lv_font_montserrat_16, 0);

        return std::make_pair(labelObj, valueObj);
    };

    // Create all status labels and store their value labels for updates
    statusLabels = {
        createStatusLabel("STATUS:", "READY"),
        createStatusLabel("AMBIENT TEMP:", "---°C"),
        createStatusLabel("OVEN TEMP:", "---°C"),
        createStatusLabel("RELAY TEMP:", "---°C"),
        createStatusLabel("RELAY FAN:", "---%"),
        createStatusLabel("DOOR POSITION:", "---%")
    };

    // Store the container for updates
    statusDisplay = statusContainer;
}

void HomeController::updateTags() {
    if (state != State::ACTIVE) return;
    
    std::vector<std::string> tags = {
        DoorService::getInstance().isFullyOpen() ? "CLOSE DOOR" : "OPEN DOOR",
        "START",
        lightsOn ? "LIGHTS OFF" : "LIGHTS ON"
    };
    layout->setBottomTags(tags);
}

void HomeController::onButton1Press() {
    if (state != State::ACTIVE) return;
    lv_async_call([](void* user_data) {
        HomeController* self = static_cast<HomeController*>(user_data);
        self->layout->setTagPressed(0, true, BUTTON_PRESS_DURATION, [self]() {
            self->toggleDoor();
            self->updateTags();
        });
    }, this);
}

void HomeController::onButton2Press() {
    if (state != State::ACTIVE) return;
    lv_async_call([](void* user_data) {
        HomeController* self = static_cast<HomeController*>(user_data);
        self->layout->setTagPressed(1, true, BUTTON_PRESS_DURATION, [self]() {
            self->selectProfile();
            self->updateTags();
        });
    }, this);
}

void HomeController::onButton3Press() {
    if (state != State::ACTIVE) return;
    lv_async_call([](void* user_data) {
        HomeController* self = static_cast<HomeController*>(user_data);
        self->layout->setTagPressed(2, true, BUTTON_PRESS_DURATION, [self]() {
            self->toggleLights();
            self->updateTags();
        });
    }, this);
}

void HomeController::onEncoderPress() {
    if (state != State::ACTIVE) return;
    lv_async_call([](void* user_data) {
        BuzzerService::getInstance().playMediumTone(100);
        HomeController* self = static_cast<HomeController*>(user_data);
        self->layout->setEncoderTagPressed(true, ENCODER_PRESS_DURATION, [self]() {
            self->navigateTo("menu");
        });
    }, this);
}

void HomeController::onEncoderUp() {
    // Remove tag selection logic - tags are handled by dedicated buttons
}

void HomeController::onEncoderDown() {
    // Remove tag selection logic - tags are handled by dedicated buttons
}

void HomeController::onEncoderLongPress() {
    if (state != State::ACTIVE) return;
    layout->setEncoderTagPressed(true, ENCODER_PRESS_DURATION, [this]() {
        // Add any long press actions here if needed
    });
}

void HomeController::toggleDoor() {
    DoorService& doorService = DoorService::getInstance();
    doorService.isFullyOpen() ? doorService.close() : doorService.open();
}

void HomeController::toggleLights() {
    lightsOn = !lightsOn;
    printf("Lights %s\n", lightsOn ? "ON" : "OFF");
}

void HomeController::selectProfile() {
    printf("Profile selection screen...\n");
}

void HomeController::openMenu() {
    printf("HomeController::openMenu\n");
    navigateToSafe("menu", 300, TransitionDirection::SLIDE_IN_LEFT);
}

void HomeController::viewDidLoad() {
    printf("HomeController::viewDidLoad\n");
}

void HomeController::viewWillAppear() {
    printf("HomeController::viewWillAppear\n");
    startUpdateTimer();
}

void HomeController::viewDidAppear() {
    printf("HomeController::viewDidAppear\n");
    updateTags();
}

void HomeController::viewWillDisappear() {
    printf("HomeController::viewWillDisappear\n");
    stopUpdateTimer();
}

void HomeController::viewDidDisappear() {
    printf("HomeController::viewDidDisappear\n");
}

void HomeController::viewWillUnload() {
    printf("HomeController::viewWillUnload\n");
    stopUpdateTimer();
    
    if (layout) {
        delete layout;
        layout = nullptr;
    }
}

void HomeController::updateStatusDisplay() {
    if (!statusDisplay || !lv_obj_is_valid(statusDisplay)) return;

    // Get current sensor readings
    const SensorState& sensorState = SensorService::getInstance().getState();
    const TemperatureState& tempState = TemperatureControlService::getInstance().getState();
    const DoorService& doorService = DoorService::getInstance();
    
    // Update status (READY/FAULT)
    const char* status = sensorState.hasError ? "FAULT" : "READY";
    lv_label_set_text(statusLabels[0].second, status);
    
    // Update temperatures
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", sensorState.ambientTemp);
    lv_label_set_text(statusLabels[1].second, tempStr);  // Ambient temp
    
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", sensorState.currentTemp);
    lv_label_set_text(statusLabels[2].second, tempStr);  // Oven temp
    
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", sensorState.ssrTemp);
    lv_label_set_text(statusLabels[3].second, tempStr);  // Relay temp
    
    // Update fan speed
    snprintf(tempStr, sizeof(tempStr), "%d%%", tempState.coolingPower);
    lv_label_set_text(statusLabels[4].second, tempStr);  // Relay fan
    
    // Update door position
    uint8_t doorPos = doorService.getPosition();
    snprintf(tempStr, sizeof(tempStr), "%d%%", doorPos);
    lv_label_set_text(statusLabels[5].second, tempStr);  // Door position
}

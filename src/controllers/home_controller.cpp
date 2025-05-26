#include "home_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include "ui/cyberpunk_theme.h"

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
    
    // Sample content
    lv_obj_t* content = layout->getContentArea();
    if (!content || !lv_obj_is_valid(content)) {
        printf("ERROR: Invalid content area in buildView\n");
        return;
    }
    
    lv_obj_t* label = lv_label_create(content);
    if (!label) {
        printf("ERROR: Failed to create label\n");
        return;
    }
    
    lv_label_set_text(label, "System Status:\nTEMP OK\nDOOR CLOSED");
    lv_obj_center(label);
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

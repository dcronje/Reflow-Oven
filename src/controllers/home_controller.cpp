#include "home_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include "ui/cyberpunk_theme.h"

HomeController& HomeController::getInstance() {
    static HomeController instance;
    return instance;
}

void HomeController::buildView(lv_obj_t* parent) {
    printf("Building Home View\n");
    
    // Initialize the theme
    CyberpunkTheme::init();
    
    layout = new CyberpunkLayout(lv_scr_act());
    printf("Layout created\n");

    // Encoder tag (vertical)
    layout->setEncoderTag("MENU", false);
    layout->setEncoderTagVisible(true);
    printf("Encoder tag set\n");

    printf("About to call updateTags\n");
    updateTags();
    printf("updateTags returned\n");
    
    // Sample content
    lv_obj_t* content = layout->getContentArea();
    printf("Content area pointer: %p\n", (void*)content);
    if (content == nullptr) {
        printf("ERROR: Content area is null!\n");
        return;
    }
    if (!lv_obj_is_valid(content)) {
        printf("ERROR: Content area is not a valid LVGL object!\n");
        return;
    }
    
    lv_obj_t* label = lv_label_create(content);
    if (label == nullptr) {
        printf("ERROR: Failed to create label!\n");
        return;
    }
    printf("Label created successfully\n");
    
    lv_label_set_text(label, "System Status:\nTEMP OK\nDOOR CLOSED");
    lv_obj_center(label);
    printf("Label text set and centered\n");
}

void HomeController::updateTags() {
    printf("updateTags: Starting\n");
    std::vector<std::string> tags = {
        DoorService::getInstance().isFullyOpen() ? "CLOSE DOOR" : "OPEN DOOR",
        "START",
        lightsOn ? "LIGHTS OFF" : "LIGHTS ON"
    };
    printf("updateTags: Tags created, calling setBottomTags\n");
    layout->setBottomTags(tags);
    printf("updateTags: setBottomTags completed\n");
}

void HomeController::showButtonPressFeedback(int index) {
    layout->setTagPressed(index, true, BUTTON_PRESS_DURATION);
}

void HomeController::showEncoderPressFeedback() {
    layout->setEncoderTagPressed(true, ENCODER_PRESS_DURATION);
}

void HomeController::onButton1Press() {
    lv_async_call([](void* user_data) {
        HomeController* self = static_cast<HomeController*>(user_data);
        self->showButtonPressFeedback(0);
        self->toggleDoor();
        self->updateTags();
    }, this);
}

void HomeController::onButton2Press() {
    lv_async_call([](void* user_data) {
        HomeController* self = static_cast<HomeController*>(user_data);
        self->showButtonPressFeedback(1);
        self->selectProfile();
        self->updateTags();
    }, this);
}

void HomeController::onButton3Press() {
    lv_async_call([](void* user_data) {
        HomeController* self = static_cast<HomeController*>(user_data);
        self->showButtonPressFeedback(2);
        self->toggleLights();
        self->updateTags();
    }, this);
}

void HomeController::onEncoderPress() {
    lv_async_call([](void* user_data) {
        BuzzerService::getInstance().playMediumTone(100);
        HomeController* self = static_cast<HomeController*>(user_data);
        self->showEncoderPressFeedback();
        self->navigateTo("menu");
        self->updateTags();
    }, this);
}

void HomeController::onEncoderUp() {
    // Remove tag selection logic - tags are handled by dedicated buttons
}

void HomeController::onEncoderDown() {
    // Remove tag selection logic - tags are handled by dedicated buttons
}

void HomeController::onEncoderLongPress() {
    // Navigate to main menu or settings
    printf("Long press: open main menu or settings\n");
    showEncoderPressFeedback();
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

void HomeController::willUnload() {
    if (layout) {
        delete layout;
        layout = nullptr;
    }
}

void HomeController::didAppear() {
    updateTags();
}

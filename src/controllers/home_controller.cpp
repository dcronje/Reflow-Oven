#include "home_controller.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"

HomeController& HomeController::getInstance() {
    static HomeController instance;
    return instance;
}

void HomeController::buildView(lv_obj_t* parent) {
    layout = new CyberpunkLayout(parent);

    // Encoder tag (vertical)
    layout->setEncoderTag("MENU", false);
    layout->setEncoderTagVisible(true);

    updateTags();
    updateButtonFeedback();

    // Sample content
    lv_obj_t* content = layout->getContentArea();
    lv_obj_t* label = lv_label_create(content);
    lv_label_set_text(label, "System Status:\nTEMP OK\nDOOR CLOSED");
    lv_obj_center(label);
}

void HomeController::updateTags() {
    std::vector<std::string> tags = {
        DoorService::getInstance().isFullyOpen() ? "CLOSE DOOR" : "OPEN DOOR",
        "SELECT PROFILE",
        lightsOn ? "LIGHTS OFF" : "LIGHTS ON"
    };
    layout->setBottomTags(tags);
}

void HomeController::updateButtonFeedback() {
    // Update visual feedback for all buttons
    for(int i = 0; i < 3; i++) {
        layout->setTagPressed(i, i == selectedIndex);
    }
}

void HomeController::showButtonPressFeedback(int index) {
    layout->setTagPressed(index, true, BUTTON_PRESS_DURATION);
}

void HomeController::showEncoderPressFeedback() {
    layout->setEncoderTagPressed(true, ENCODER_PRESS_DURATION);
}

void HomeController::onEncoderPress() {
    BuzzerService::getInstance().playMediumTone(100);
    showEncoderPressFeedback();

    // Show feedback for the selected button
    showButtonPressFeedback(selectedIndex);

    switch (selectedIndex) {
        case 0: toggleDoor(); break;
        case 1: selectProfile(); break;
        case 2: toggleLights(); break;
    }
    updateTags();
}

void HomeController::onEncoderUp() {
    if (--selectedIndex < 0) selectedIndex = 2;
    BuzzerService::getInstance().playMediumTone(100);
    updateButtonFeedback();
}

void HomeController::onEncoderDown() {
    if (++selectedIndex > 2) selectedIndex = 0;
    BuzzerService::getInstance().playMediumTone(100);
    updateButtonFeedback();
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
    updateButtonFeedback();
}

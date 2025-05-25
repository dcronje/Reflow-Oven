#pragma once

#include "core/controller.h"
#include "ui/cyberpunk_layout.h"

class HomeController : public Controller {
public:
    static HomeController& getInstance();
    void buildView(lv_obj_t* parent) override;
    void willUnload() override;
    void didAppear() override;

    void onEncoderUp() override;
    void onEncoderDown() override;
    void onEncoderPress() override;
    void onEncoderLongPress() override;

    // Button event handlers
    void onButton1Press() override;
    void onButton2Press() override;
    void onButton3Press() override;

private:
    CyberpunkLayout* layout = nullptr;
    bool lightsOn = false;
    bool encoderPressed = false;

    // Button press feedback duration
    static constexpr uint32_t BUTTON_PRESS_DURATION = 200;
    static constexpr uint32_t ENCODER_PRESS_DURATION = 150;

    void updateTags();
    void toggleDoor();
    void toggleLights();
    void selectProfile();
    
    // Visual feedback methods
    void showEncoderPressFeedback();
    void showButtonPressFeedback(int index);
};

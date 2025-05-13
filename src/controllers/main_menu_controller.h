#pragma once

#include "core/controller.h"
#include "lvgl.h"

class MainMenuController : public Controller {
public:
    static MainMenuController& getInstance();

    // Controller interface methods
    void render() override;
    void init() override;
    void onEncoderPress() override;
    void onEncoderUp() override;
    void onEncoderDown() override;
    void onEncoderLongPress() override;

    // UI action methods
    void selectReflowCurve();
    void startReflow();
    void calibrate();
    void openSettings();
    void toggleDoor();

private:
    MainMenuController() = default;
    
    // UI elements
    lv_obj_t* menu = nullptr;
    int selectedIndex = 0;
    
    // Helper methods
    void updateButtonFocus();
};

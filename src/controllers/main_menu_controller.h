#pragma once

#include <vector>
#include "core/controller.h"
#include "lvgl.h"

class MainMenuController : public Controller {
public:
    static MainMenuController& getInstance();

    void buildView(lv_obj_t* parent) override;
    void init() override;
    void willUnload() override;

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

    lv_obj_t* menu = nullptr;
    std::vector<lv_obj_t*> buttons;
    int selectedIndex = 0;

    void updateButtonFocus(bool animated = true);
};

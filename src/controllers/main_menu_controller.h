#pragma once

#include "base_controller.h"

class MainMenuController : public BaseController {
public:
    static MainMenuController& getInstance();

    void init() override;

    void selectReflowCurve(); // Called by UI when user selects reflow curve
    void startReflow();
    void calibrate();
    void openSettings();
    void toggleDoor();

private:
    MainMenuController() = default;
};

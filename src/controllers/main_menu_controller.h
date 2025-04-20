#pragma once

#include "base_controller.h"

class MainMenuController : public BaseController {
public:
    static MainMenuController& getInstance();

    void registerViews(UIViewService& viewService) override;

    void selectReflowCurve(); // Called by UI when user selects reflow curve
    void startReflow();
    void calibrate();
    void openSettings();

private:
    MainMenuController() = default;
};

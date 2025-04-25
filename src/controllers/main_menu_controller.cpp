#include "main_menu_controller.h"
#include "ui/views/main_menu_view.h"
#include "ui/views/reflow_curve_selection_view.h"
#include "services/ui_view_service.h"
#include "services/door_service.h"

MainMenuController& MainMenuController::getInstance() {
    static MainMenuController instance;
    return instance;
}

void MainMenuController::init() {
    auto& ui = UIViewService::getInstance();
    ui.registerView("main-menu", std::make_unique<MainMenuView>());
    ui.registerView("select-curve", std::make_unique<ReflowCurveSelectionView>());
}

void MainMenuController::selectReflowCurve() {
    UIViewService::getInstance().showView("select-curve");
}

void MainMenuController::startReflow() {
    // TODO: Switch to ReflowController view
}

void MainMenuController::calibrate() {
    // TODO: Show calibration view
}

void MainMenuController::openSettings() {
    // TODO: Show settings view
}

void MainMenuController::toggleDoor() {
    if (DoorService::getInstance().isFullyOpen()) {
        DoorService::getInstance().setPosition(0); // Close door
    } else {
        DoorService::getInstance().setPosition(100); // Open door
    }
}

#include "main_menu_controller.h"
#include "ui/views/main_menu_view.h"
#include "ui/views/reflow_curve_selection_view.h"

MainMenuController& MainMenuController::getInstance() {
    static MainMenuController instance;
    return instance;
}

void MainMenuController::registerViews(UIViewService& ui) {
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

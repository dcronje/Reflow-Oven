#include "main_menu_view.h"
#include "types/display.h"
#include "controllers/main_menu_controller.h"
#include "controllers/calibration_controller.h"
#include "services/door_service.h"

void MainMenuView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
    menu.setTitle("Main Menu");

    menu.clearItems();
    menu.addItem(MenuItemElement("Start Reflow", [] {
        MainMenuController::getInstance().startReflow();
    }, std::nullopt, 
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));

    menu.addItem(MenuItemElement("Door", [] {
        if (!CalibrationController::getInstance().isDoorCalibrated()) {
            MainMenuController::getInstance().calibrate();
        } else {
            MainMenuController::getInstance().toggleDoor();
        }
    }, std::nullopt,
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));

    menu.addItem(MenuItemElement("Calibrate", [] {
        MainMenuController::getInstance().calibrate();
    }, std::nullopt, 
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));

    menu.addItem(MenuItemElement("Settings", [] {
        MainMenuController::getInstance().openSettings();
    }, std::nullopt,
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));
}

void MainMenuView::render(DisplaySSD1331_96x64x8_SPI& display) {
    // Update door menu item label based on current state
    if (!CalibrationController::getInstance().isDoorCalibrated()) {
        menu.updateItemLabel(1, "Calibrate Door");
    } else {
        menu.updateItemLabel(1, DoorService::getInstance().isFullyOpen() ? "Close Door" : "Open Door");
    }

    menu.render(canvas);
    display.drawCanvas(0, 0, canvas);
}

void MainMenuView::handleEncoderUp() {
    menu.moveUp();
}

void MainMenuView::handleEncoderDown() {
    menu.moveDown();
}

void MainMenuView::handleEncoderPress() {
    menu.select();
}

void MainMenuView::handleEncoderLongPress() {
    // Optional: navigate to previous view or home screen
}

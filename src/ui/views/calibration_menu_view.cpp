// ui/views/calibration_menu_view.cpp

#include "calibration_menu_view.h"
#include "controllers/calibration_controller.h"
#include "types/display.h"

void CalibrationMenuView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
    menu.setTitle("Calibration");

    menu.clearItems();
    menu.addItem(MenuItemElement("Sensor Calibration", [] {
        CalibrationController::getInstance().startSensorCalibration();
    }, std::nullopt,
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));

    menu.addItem(MenuItemElement("Thermal Calibration", [] {
        CalibrationController::getInstance().startThermalCalibration();
    }, std::nullopt,
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));

    menu.addItem(MenuItemElement("View Summary", [] {
        CalibrationController::getInstance().viewCalibrationSummary();
    }, std::nullopt,
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));

    menu.addItem(MenuItemElement("Back", [] {
        CalibrationController::getInstance().returnToMainMenu();
    }, std::nullopt,
    { .backgroundColor = BLACK, .textColor = WHITE },
    { .backgroundColor = GREEN, .textColor = BLACK }));
}

void CalibrationMenuView::render(DisplaySSD1331_96x64x8_SPI& display) {
    menu.render(canvas);
    display.drawCanvas(0, 0, canvas);
}

void CalibrationMenuView::handleEncoderUp() {
    menu.moveUp();
}

void CalibrationMenuView::handleEncoderDown() {
    menu.moveDown();
}

void CalibrationMenuView::handleEncoderPress() {
    menu.select();
}

void CalibrationMenuView::handleEncoderLongPress() {
    // Optional: navigate to previous view or home screen
}

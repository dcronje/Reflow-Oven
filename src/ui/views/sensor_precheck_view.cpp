#include "sensor_precheck_view.h"
#include "controllers/calibration_controller.h"
#include <stdio.h>
#include <cmath>

void SensorPrecheckView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

bool SensorPrecheckView::checkTemperatureDifference() const {
    auto& controller = CalibrationController::getInstance();
    return controller.checkTemperatureDifference();
}

void SensorPrecheckView::drawTemperatureInfo(float frontTemp, float backTemp, float ambientTemp) {
    char tempInfo[32];
    snprintf(tempInfo, sizeof(tempInfo), "Front: %.1fC", frontTemp);
    canvas.printFixed(0, 12, tempInfo, STYLE_NORMAL);
    
    snprintf(tempInfo, sizeof(tempInfo), "Back: %.1fC", backTemp);
    canvas.printFixed(0, 24, tempInfo, STYLE_NORMAL);
    
    snprintf(tempInfo, sizeof(tempInfo), "Ambient: %.1fC", ambientTemp);
    canvas.printFixed(0, 36, tempInfo, STYLE_NORMAL);
}

void SensorPrecheckView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = CalibrationController::getInstance();
    const auto& data = controller.getCalibrationData();
    const auto& temps = controller.getCurrentTemperatures();

    // Title
    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, "Sensor Offsets", STYLE_BOLD);

    // Current offsets
    char offsetInfo[32];
    snprintf(offsetInfo, sizeof(offsetInfo), "Front: %.1fC", data.frontSensorOffset);
    canvas.printFixed(0, 12, offsetInfo, STYLE_NORMAL);
    
    snprintf(offsetInfo, sizeof(offsetInfo), "Back: %.1fC", data.backSensorOffset);
    canvas.printFixed(0, 24, offsetInfo, STYLE_NORMAL);

    // Current temperatures
    drawTemperatureInfo(temps.front, temps.back, temps.ambient);

    // Instructions
    if (checkTemperatureDifference()) {
        canvas.setColor(GREEN);
        canvas.printFixed(0, 48, "Press to calibrate", STYLE_NORMAL);
    } else {
        canvas.setColor(RED);
        canvas.printFixed(0, 48, "Oven too hot!", STYLE_NORMAL);
    }
    
    canvas.setColor(WHITE);
    canvas.printFixed(0, 56, "Long press to go back", STYLE_NORMAL);

    display.drawCanvas(0, 0, canvas);
}

void SensorPrecheckView::handleEncoderPress() {
    auto& controller = CalibrationController::getInstance();
    controller.startSensorCalibration();
}

void SensorPrecheckView::handleEncoderLongPress() {
    auto& controller = CalibrationController::getInstance();
    controller.returnToCalibrationMenu();
}

void SensorPrecheckView::handleEncoderUp() {
    // No action needed
}

void SensorPrecheckView::handleEncoderDown() {
    // No action needed
} 
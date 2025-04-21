#include "sensor_calibration_view.h"
#include "controllers/calibration_controller.h"

void SensorCalibrationView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

void SensorCalibrationView::drawProgressBar(int x, int y, int width, int height, float progress) {
    // Draw background
    canvas.setColor(GREY);
    canvas.fillRect(x, y, width, height);
    
    // Draw progress
    canvas.setColor(GREEN);
    int progressWidth = static_cast<int>(width * progress);
    canvas.fillRect(x, y, progressWidth, height);
}

void SensorCalibrationView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = CalibrationController::getInstance();
    const auto& state = controller.getCurrentState();

    // Title
    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, "Sensor Calibration", STYLE_BOLD);

    // Show error if present
    if (state.hasError && state.errorMessage) {
        canvas.setColor(RED);
        canvas.printFixed(0, 12, state.errorMessage, STYLE_NORMAL);
        canvas.setColor(WHITE);
    }

    // Temperature information
    char tempInfo[16];
    snprintf(tempInfo, sizeof(tempInfo), "Temp: %dC", static_cast<int>(state.currentTemp));
    canvas.printFixed(0, 24, tempInfo, STYLE_NORMAL);

    // Overall progress bar
    drawProgressBar(0, 34, 96, 8, state.progress);

    // Time remaining
    char timeInfo[16];
    snprintf(timeInfo, sizeof(timeInfo), "%d sec", static_cast<int>(state.timeRemainingMs / 1000));
    canvas.printFixed(0, 44, timeInfo, STYLE_NORMAL);

    display.drawCanvas(0, 0, canvas);
}

void SensorCalibrationView::handleEncoderPress() {
    // No action needed
}

void SensorCalibrationView::handleEncoderLongPress() {
    // Cancel calibration
    CalibrationController::getInstance().stopCalibration();
}

void SensorCalibrationView::handleEncoderUp() {
    // No action needed
}

void SensorCalibrationView::handleEncoderDown() {
    // No action needed
} 
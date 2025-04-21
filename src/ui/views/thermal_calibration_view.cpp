#include "thermal_calibration_view.h"
#include "controllers/calibration_controller.h"

void ThermalCalibrationView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

void ThermalCalibrationView::drawProgressBar(int x, int y, int width, int height, float progress) {
    // Draw background
    canvas.setColor(GREY);
    canvas.fillRect(x, y, width, height);
    
    // Draw progress
    canvas.setColor(GREEN);
    int progressWidth = static_cast<int>(width * progress);
    canvas.fillRect(x, y, progressWidth, height);
}

void ThermalCalibrationView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = CalibrationController::getInstance();
    const auto& state = controller.getCurrentState();

    // Title based on current phase
    canvas.setColor(WHITE);
    const char* title = "Thermal Calibration";
    if (state.phase == CalibrationPhase::HEATING_CALIBRATION) {
        title = "Testing Heating";
    } else if (state.phase == CalibrationPhase::COOLING_CALIBRATION) {
        title = "Testing Cooling";
    }
    canvas.printFixed(0, 0, title, STYLE_BOLD);

    // Show error if present
    if (state.hasError && state.errorMessage) {
        canvas.setColor(RED);
        canvas.printFixed(0, 12, state.errorMessage, STYLE_NORMAL);
        canvas.setColor(WHITE);
    } else {
        // Power level if in heating/cooling phase
        if (state.phase == CalibrationPhase::HEATING_CALIBRATION || 
            state.phase == CalibrationPhase::COOLING_CALIBRATION) {
            char powerInfo[16];
            snprintf(powerInfo, sizeof(powerInfo), "at %d%%", static_cast<int>(state.progress * 100));
            canvas.printFixed(0, 12, powerInfo, STYLE_NORMAL);
        }
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

void ThermalCalibrationView::handleEncoderPress() {
    // No action needed
}

void ThermalCalibrationView::handleEncoderLongPress() {
    // Cancel calibration
    CalibrationController::getInstance().stopCalibration();
}

void ThermalCalibrationView::handleEncoderUp() {
    // No action needed
}

void ThermalCalibrationView::handleEncoderDown() {
    // No action needed
} 
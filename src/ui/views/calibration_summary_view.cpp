#include "calibration_summary_view.h"
#include "controllers/calibration_controller.h"
#include "services/ui_view_service.h"

void CalibrationSummaryView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

void CalibrationSummaryView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = CalibrationController::getInstance();
    const auto& data = controller.getCalibrationData();
    const auto& thermal = controller.getThermalSummary();

    // Header
    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, "Calibration Summary", STYLE_BOLD);

    // Sensor Calibration
    canvas.setColor(GREEN);
    canvas.printFixed(0, 12, "Sensor Offsets:", STYLE_NORMAL);
    canvas.printFixed(0, 22, ("Front: " + std::to_string(data.frontSensorOffset) + "C").c_str(), STYLE_NORMAL);
    canvas.printFixed(0, 32, ("Back: " + std::to_string(data.backSensorOffset) + "C").c_str(), STYLE_NORMAL);

    // Thermal Calibration
    canvas.setColor(BLUE);
    canvas.printFixed(0, 44, "Heating Rates:", STYLE_NORMAL);
    canvas.printFixed(0, 54, ("100%: " + std::to_string(thermal.heatingRates[9]) + "C/s").c_str(), STYLE_NORMAL);

    display.drawCanvas(0, 0, canvas);
}

void CalibrationSummaryView::handleEncoderPress() {
    // Return to calibration menu
    UIViewService::getInstance().showView("calibration-menu");
}

void CalibrationSummaryView::handleEncoderLongPress() {
    // Return to main menu
    UIViewService::getInstance().showView("main-menu");
}

void CalibrationSummaryView::handleEncoderUp() {
    // Optional: Scroll through more data if needed
}

void CalibrationSummaryView::handleEncoderDown() {
    // Optional: Scroll through more data if needed
} 
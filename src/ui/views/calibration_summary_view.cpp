#include "calibration_summary_view.h"
#include "controllers/calibration_controller.h"

void CalibrationSummaryView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
    selectedBar = 0;
}

void CalibrationSummaryView::drawBarGraph(const float* rates, uint8_t color) {
    const int graphWidth = 80;
    const int graphHeight = 40;
    const int startX = 8;
    const int startY = 20;
    const int barWidth = 6;
    const int barSpacing = 2;

    // Find max rate for scaling
    float maxRate = 0;
    for (int i = 0; i < 10; i++) {
        if (rates[i] > maxRate) maxRate = rates[i];
    }

    // Draw bars
    for (int i = 0; i < 10; i++) {
        int barHeight = static_cast<int>((rates[i] / maxRate) * graphHeight);
        int x = startX + (i * (barWidth + barSpacing));
        int y = startY + (graphHeight - barHeight);
        
        // Highlight selected bar
        if (i == selectedBar) {
            canvas.setColor(WHITE);
            canvas.drawRect(x - 1, y - 1, barWidth + 2, barHeight + 2);
        }
        
        canvas.setColor(color);
        canvas.fillRect(x, y, barWidth, barHeight);
    }
}

void CalibrationSummaryView::drawSelectedBarInfo(int power, float rate) {
    // Format: "XX%: Y.YC/s"
    char powerStr[5];  // Increased size to include null terminator
    char rateStr[7];   // Increased size to include null terminator
    
    // Convert power to string (10-100)
    if (power < 100) {
        powerStr[0] = '0' + (power / 10);
        powerStr[1] = '0' + (power % 10);
        powerStr[2] = '%';
        powerStr[3] = '\0';
    } else {
        powerStr[0] = '1';
        powerStr[1] = '0';
        powerStr[2] = '0';
        powerStr[3] = '%';
        powerStr[4] = '\0';
    }
    
    // Convert rate to string with one decimal place
    int whole = static_cast<int>(rate);
    int decimal = static_cast<int>((rate - whole) * 10);
    rateStr[0] = '0' + whole;
    rateStr[1] = '.';
    rateStr[2] = '0' + decimal;
    rateStr[3] = 'C';
    rateStr[4] = '/';
    rateStr[5] = 's';
    rateStr[6] = '\0';
    
    // Combine and display
    char info[13];  // Increased size to include null terminator
    info[0] = powerStr[0];
    info[1] = powerStr[1];
    info[2] = powerStr[2];
    info[3] = powerStr[3];
    info[4] = ':';
    info[5] = ' ';
    info[6] = rateStr[0];
    info[7] = rateStr[1];
    info[8] = rateStr[2];
    info[9] = rateStr[3];
    info[10] = rateStr[4];
    info[11] = rateStr[5];
    info[12] = '\0';
    
    canvas.setColor(WHITE);
    canvas.printFixed(0, 52, info, STYLE_NORMAL);
}

void CalibrationSummaryView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = CalibrationController::getInstance();
    const auto& thermal = controller.getThermalSummary();

    // Title
    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, currentMode == DisplayMode::HEATING ? "Heating Rates" : "Cooling Rates", STYLE_BOLD);

    // Draw the appropriate graph
    if (currentMode == DisplayMode::HEATING) {
        drawBarGraph(thermal.heatingRates, RED);
        drawSelectedBarInfo((selectedBar + 1) * 10, thermal.heatingRates[selectedBar]);
    } else {
        drawBarGraph(thermal.coolingRates, BLUE);
        drawSelectedBarInfo((selectedBar + 1) * 10, thermal.coolingRates[selectedBar]);
    }

    display.drawCanvas(0, 0, canvas);
}

void CalibrationSummaryView::handleEncoderPress() {
    // Switch between heating and cooling modes
    currentMode = (currentMode == DisplayMode::HEATING) ? DisplayMode::COOLING : DisplayMode::HEATING;
}

void CalibrationSummaryView::handleEncoderLongPress() {
    // Return to calibration menu
    CalibrationController::getInstance().stopCalibration();
}

void CalibrationSummaryView::handleEncoderUp() {
    // Move to next bar
    selectedBar = (selectedBar + 1) % 10;
}

void CalibrationSummaryView::handleEncoderDown() {
    // Move to previous bar
    selectedBar = (selectedBar - 1 + 10) % 10;
} 
#include "calibration_summary_view.h"
#include "controllers/calibration_controller.h"

void CalibrationSummaryView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
    selectedBar = 0;
    selectedTempPoint = 1;  // Start with middle temperature point (100°C)
}

void CalibrationSummaryView::drawBarGraph(const std::array<float, 10>& rates, uint8_t color) {
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

    // Title with temperature point
    canvas.setColor(WHITE);
    char title[32];
    const char* mode = currentMode == DisplayMode::HEATING ? "Heating" : "Cooling";
    const char* tempPoint = selectedTempPoint == 0 ? "25°C" : 
                           selectedTempPoint == 1 ? "100°C" : "200°C";
    snprintf(title, sizeof(title), "%s Rates at %s", mode, tempPoint);
    canvas.printFixed(0, 0, title, STYLE_BOLD);

    // Get rates and draw the appropriate graph
    float powerPercent = (selectedBar + 1) * 10.0f;
    float rate;

    if (currentMode == DisplayMode::HEATING) {
        auto rates = thermal.getHeatingRatesAtTemp(selectedTempPoint);
        drawBarGraph(rates, RED);
        rate = thermal.getHeatingRateAtTempAndPowerPercent(selectedTempPoint, powerPercent);
    } else {
        auto rates = thermal.getCoolingRatesAtTemp(selectedTempPoint);
        drawBarGraph(rates, BLUE);
        rate = thermal.getCoolingRateAtTempAndPowerPercent(selectedTempPoint, powerPercent);
    }

    drawSelectedBarInfo(static_cast<int>(powerPercent), rate);
    display.drawCanvas(0, 0, canvas);
}

void CalibrationSummaryView::handleEncoderPress() {
    // Switch between heating and cooling modes
    currentMode = (currentMode == DisplayMode::HEATING) ? DisplayMode::COOLING : DisplayMode::HEATING;
}

void CalibrationSummaryView::handleEncoderLongPress() {
    // Return to calibration menu
    CalibrationController::getInstance().returnToCalibrationMenu();
}

void CalibrationSummaryView::handleEncoderUp() {
    // Move to next bar, wrapping to next temperature point if needed
    selectedBar++;
    if (selectedBar >= 10) {
        selectedBar = 0;
        selectedTempPoint = (selectedTempPoint + 1) % 3;  // Cycle through 0, 1, 2
    }
}

void CalibrationSummaryView::handleEncoderDown() {
    // Move to previous bar, wrapping to previous temperature point if needed
    selectedBar--;
    if (selectedBar < 0) {
        selectedBar = 9;
        selectedTempPoint = (selectedTempPoint - 1 + 3) % 3;  // Cycle through 0, 1, 2
    }
} 
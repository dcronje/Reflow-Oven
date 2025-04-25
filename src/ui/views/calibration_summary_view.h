#pragma once

#include "ui_view.h"
#include "types/display.h"
#include <array>

class CalibrationSummaryView : public UIView {
public:
    void init() override;
    void render(DisplaySSD1331_96x64x8_SPI& display) override;

    void handleEncoderUp() override;
    void handleEncoderDown() override;
    void handleEncoderPress() override;
    void handleEncoderLongPress() override;

private:
    void drawBarGraph(const std::array<float, 10>& rates, uint8_t color);
    void drawSelectedBarInfo(int power, float rate);

    enum class DisplayMode {
        HEATING,
        COOLING
    } currentMode = DisplayMode::HEATING;

    enum class SelectionState {
        TEMP_POINT,
        BAR
    } selectionState = SelectionState::TEMP_POINT;

    int selectedBar = 0;
    int selectedTempPoint = 1;  // Start with middle temperature point (100°C)
    NanoCanvas8 canvas = NanoCanvas8(96, 64, buffer);
    uint8_t buffer[96 * 64];
}; 
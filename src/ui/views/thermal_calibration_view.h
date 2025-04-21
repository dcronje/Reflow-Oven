#pragma once

#include "ui_view.h"
#include "types/display.h"

class ThermalCalibrationView : public UIView {
public:
    void init() override;
    void render(DisplaySSD1331_96x64x8_SPI& display) override;

    void handleEncoderUp() override;
    void handleEncoderDown() override;
    void handleEncoderPress() override;
    void handleEncoderLongPress() override;

private:
    void drawProgressBar(int x, int y, int width, int height, float progress);

    NanoCanvas8 canvas = NanoCanvas8(96, 64, buffer);
    uint8_t buffer[96 * 64];
}; 
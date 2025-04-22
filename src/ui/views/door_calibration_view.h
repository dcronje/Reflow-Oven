#pragma once

#include "ui_view.h"
#include "types/display.h"
#include "controllers/calibration_controller.h"

class DoorCalibrationView : public UIView {
public:
    void init() override;
    void render(DisplaySSD1331_96x64x8_SPI& display) override;

    void handleEncoderUp() override;
    void handleEncoderDown() override;
    void handleEncoderPress() override;
    void handleEncoderLongPress() override;

private:
    void drawProgressBar(int x, int y, int width, int height, float progress);
    void drawStepIndicator(int step);
    void drawCurrentPosition(float position);

    enum class CalibrationStep {
        OPEN,
        CLOSED,
        COMPLETE
    } currentStep = CalibrationStep::OPEN;

    NanoCanvas8 canvas = NanoCanvas8(96, 64, buffer);
    uint8_t buffer[96 * 64];
}; 
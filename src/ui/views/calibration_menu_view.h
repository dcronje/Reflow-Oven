#pragma once

#include "ui_view.h"
#include "ui/elements/menu_element.h"

class CalibrationMenuView : public UIView {
public:
    void init() override;
    void render(DisplaySSD1331_96x64x8_SPI& display) override;

    void handleEncoderUp() override;
    void handleEncoderDown() override;
    void handleEncoderPress() override;
    void handleEncoderLongPress() override;

private:
    NanoCanvas8 canvas = NanoCanvas8(96, 64, buffer);
    uint8_t buffer[96 * 64];

    MenuElement menu;
};


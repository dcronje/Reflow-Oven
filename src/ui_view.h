#pragma once

#include <lcdgfx.h>

class UIView {
public:
    virtual ~UIView() = default;

    virtual void init() = 0;
    virtual void render(DisplaySSD1331_96x64x8_SPI& display) = 0;

    virtual void handleEncoderUp() = 0;
    virtual void handleEncoderDown() = 0;
    virtual void handleEncoderPress() = 0;
    virtual void handleEncoderLongPress() = 0;
};


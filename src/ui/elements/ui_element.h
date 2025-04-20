#pragma once

#include <lcdgfx.h>

class UIElement {
public:
    virtual ~UIElement() = default;

    virtual void render(NanoCanvas8& canvas) = 0;

    // Optional interactive element hooks
    virtual void onSelect() {}
    virtual void onHighlight() {}

    virtual void setFocused(bool focused) { isFocused = focused; }
    virtual bool getFocused() const { return isFocused; }

protected:
    bool isFocused = false;
};


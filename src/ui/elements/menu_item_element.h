#pragma once

#include <string>
#include <functional>
#include <optional>

struct MenuItemStyle {
    uint8_t backgroundColor;
    uint8_t textColor;
};

struct MenuItemElement {
    std::string label;
    std::function<void()> onSelect;

    std::optional<char> iconChar;

    MenuItemStyle normalStyle = { 0x00, 0xFF };   // Black bg, white text
    MenuItemStyle highlightStyle = { 0xFF, 0x00 }; // White bg, black text

    MenuItemElement(
        const std::string& label,
        std::function<void()> onSelect,
        std::optional<char> iconChar = std::nullopt,
        MenuItemStyle normalStyle = { 0x00, 0xFF },
        MenuItemStyle highlightStyle = { 0xFF, 0x00 }
    ) : label(label),
        onSelect(onSelect),
        iconChar(iconChar),
        normalStyle(normalStyle),
        highlightStyle(highlightStyle) {}

    void setLabel(const std::string& newLabel) {
        label = newLabel;
    }
};

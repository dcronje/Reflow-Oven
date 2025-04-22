#include "menu_element.h"

void MenuElement::addItem(const MenuItemElement& item) {
    items.push_back(item);
}

void MenuElement::clearItems() {
    items.clear();
    selectedIndex = 0;
}

void MenuElement::moveUp() {
    if (selectedIndex > 0) selectedIndex--;
}

void MenuElement::moveDown() {
    if (selectedIndex < (int)items.size() - 1) selectedIndex++;
}

void MenuElement::select() {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        items[selectedIndex].onSelect();
        flashOnSelect = true;
        flashCountdown = 2; // Flash for 2 frames
    }
}

void MenuElement::setTitle(const std::string& t, uint8_t color) {
    title = t;
    titleColor = color;
}

void MenuElement::render(NanoCanvas8& canvas) {
    canvas.clear();

    // Title
    canvas.setColor(titleColor);
    canvas.setFixedFont(ssd1306xled_font6x8);
    canvas.printFixed(0, 0, title.c_str(), STYLE_NORMAL);

    const int itemStartY = 12;
    const int lineHeight = 10;

    for (size_t i = 0; i < items.size(); ++i) {
        auto& item = items[i];
        bool isSelected = ((int)i == selectedIndex);
        bool shouldFlash = (isSelected && flashOnSelect && flashCountdown > 0);

        // Choose style
        auto style = isSelected ? item.highlightStyle : item.normalStyle;
        if (shouldFlash) std::swap(style.backgroundColor, style.textColor);

        // Background
        canvas.setColor(style.backgroundColor);
        canvas.drawRect(0, itemStartY + i * lineHeight - 1, 96, lineHeight);

        // Text
        canvas.setColor(style.textColor);
        int xOffset = 4;

        if (item.iconChar.has_value()) {
            char icon[2] = { item.iconChar.value(), '\0' };
            canvas.printFixed(xOffset, itemStartY + i * lineHeight, icon, STYLE_NORMAL);
            xOffset += 8; // leave space after icon
        }

        canvas.printFixed(xOffset, itemStartY + i * lineHeight, item.label.c_str(), STYLE_NORMAL);
    }

    if (flashCountdown > 0) {
        flashCountdown--;
        if (flashCountdown == 0) {
            flashOnSelect = false;
        }
    }
}

void MenuElement::updateItemLabel(int index, const std::string& newLabel) {
    if (index >= 0 && index < items.size()) {
        items[index].setLabel(newLabel);
    }
}

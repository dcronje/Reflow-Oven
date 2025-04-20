#pragma once

#include "ui_element.h"
#include "menu_item_element.h"
#include <vector>

class MenuElement : public UIElement {
public:
    void addItem(const MenuItemElement& item);
    void clearItems();

    void moveUp();
    void moveDown();
    void select();

    void render(NanoCanvas8& canvas) override;

    void setTitle(const std::string& title, uint8_t color = 0xFF);

private:
    std::vector<MenuItemElement> items;
    int selectedIndex = 0;

    std::string title;
    uint8_t titleColor = 0xFF;
    bool flashOnSelect = false;
    int flashCountdown = 0;
};


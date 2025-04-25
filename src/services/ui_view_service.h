#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include <map>
#include <memory>
#include <string>

#include "ui_view.h"
#include <lcdgfx.h>
#include <lcdgfx_gui.h>

class UIViewService {
public:
    static UIViewService& getInstance();

    void init();
    void registerView(const std::string& name, std::unique_ptr<UIView> view);
    void showView(const std::string& name);

    // Forwarded from InteractionService
    void handleEncoderUp();
    void handleEncoderDown();
    void handleEncoderPress();
    void handleEncoderLongPress();

private:
    UIViewService();
    static void renderTaskEntry(void* param);
    void renderLoop();

    TaskHandle_t renderTask = nullptr;

    std::map<std::string, std::unique_ptr<UIView>> views;
    UIView* currentView = nullptr;
    std::string currentViewName;

    DisplaySSD1331_96x64x8_SPI display;
};


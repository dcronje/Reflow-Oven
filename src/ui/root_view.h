#pragma once

#include "lvgl.h"
#include "core/controller_collection.h"
#include <memory>

class RootView {
public:
    RootView();
    void init(lv_display_t* display);
    void update();

    // Input handling
    void handleEncoderUp();
    void handleEncoderDown();
    void handleEncoderPress();
    void handleEncoderLongPress();
    void handleTopButtonPress();
    void handleTopButtonLongPress();
    void handleBottomButtonPress();
    void handleBottomButtonLongPress();

private:
    lv_display_t* display;
    lv_obj_t* rootScreen;
    lv_obj_t* topBar;
    lv_obj_t* sideBar;

    std::unique_ptr<ControllerCollection> controllerCollection;
};

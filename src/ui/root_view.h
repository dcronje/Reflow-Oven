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
    void scheduleEncoderUpHandler(uint32_t delayMs);
    static void encoderUpCallback(struct _lv_timer_t* timer);
    
    void scheduleEncoderDownHandler(uint32_t delayMs);
    static void encoderDownCallback(struct _lv_timer_t* timer);
    
    void scheduleEncoderPressHandler(uint32_t delayMs);  
    static void encoderPressCallback(struct _lv_timer_t* timer);
    
    void scheduleEncoderLongPressHandler(uint32_t delayMs);
    static void encoderLongPressCallback(struct _lv_timer_t* timer);
    
    void handleTopButtonPress();
    void handleTopButtonLongPress();
    void handleBottomButtonPress();
    void handleBottomButtonLongPress();

    std::unique_ptr<ControllerCollection> controllerCollection;

private:
    lv_display_t* display;
    lv_obj_t* rootScreen;
    lv_obj_t* topBar;
    lv_obj_t* sideBar;
    lv_obj_t* contentArea;

    
};

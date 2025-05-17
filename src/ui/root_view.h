#pragma once

#include "lvgl.h"
#include "core/controller_collection.h"
#include "core/encoder_events_interface.h"
#include <memory>

class RootView : public EncoderEventsInterface {
public:
    RootView();
    void init(lv_display_t* display);
    void update();

    // Top-level controller collection - made public for UIViewService access
    std::unique_ptr<ControllerCollection> controllerCollection;

    // EncoderEventsInterface implementation
    void handleEncoderUp() override;
    void handleEncoderDown() override;
    void handleEncoderPress() override;
    void handleEncoderLongPress() override;

    // Keep button event handlers
    void handleTopButtonPress();
    void handleTopButtonLongPress();
    void handleBottomButtonPress();
    void handleBottomButtonLongPress();

private:
    lv_display_t* display;
    lv_obj_t* rootScreen;
    lv_obj_t* topBar;
    lv_obj_t* sideBar;
    lv_obj_t* contentArea;
};

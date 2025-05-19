#pragma once

#include "lvgl.h"
#include "core/controller_collection.h"
#include "core/input_events_interface.h"
#include <memory>

class RootView : public InputEventsInterface {
public:
    RootView();
    void init(lv_display_t* display);
    void update();

    // Top-level controller collection - made public for UIViewService access
    std::unique_ptr<ControllerCollection> controllerCollection;

    // InputEventsInterface implementation
    void handleEncoderUp() override;
    void handleEncoderDown() override;
    void handleButtonPress(int buttonId) override;
    void handleButtonLongPress(int buttonId) override;

private:
    lv_display_t* display;
    lv_obj_t* rootScreen;
    lv_obj_t* topBar;
    lv_obj_t* sideBar;
    lv_obj_t* contentArea;
};

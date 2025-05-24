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

    // Button event handlers from EncoderEventsInterface
    void handleButton1Press() override;
    void handleButton1LongPress() override;
    void handleButton2Press() override;
    void handleButton2LongPress() override;
    void handleButton3Press() override;
    void handleButton3LongPress() override;
    void handleButton4Press() override;
    void handleButton4LongPress() override;

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

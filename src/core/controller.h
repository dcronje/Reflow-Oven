#pragma once

#include <string>
#include "lvgl.h"
#include "types/transitions.h"
#include "core/input_events_interface.h"

// Forward declarations
class ControllerCollection;

class Controller {
protected:
    lv_obj_t* rootView = nullptr;
    ControllerCollection* controllerCollection = nullptr;

public:
    virtual ~Controller() = default;

    void render(lv_obj_t* parent);

    // Responsible for creating the view if needed
    virtual void buildView(lv_obj_t* parent) = 0;

    // Return the root LVGL object of this controller
    lv_obj_t* getView() const { return rootView; }

    // Called when this controller is about to be hidden or removed
    virtual void willUnload() {}

    // Called when this controller becomes visible
    virtual void didAppear() {}

    // Input event handlers
    virtual void onEncoderUp() {}
    virtual void onEncoderDown() {}
    virtual void onButtonPress(int buttonId) {}
    virtual void onButtonLongPress(int buttonId) {}

    // Called once at startup
    virtual void init() {}

    void setControllerCollection(ControllerCollection* collection) {
        controllerCollection = collection;
    }

    void navigateTo(const std::string& controllerId, uint32_t duration = 300, TransitionDirection direction = TransitionDirection::SLIDE_IN_LEFT);
    void invalidateView();
};

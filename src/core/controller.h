// controller.h
#pragma once

#include "view.h"
#include <lvgl.h>
#include <string>
#include "types/transitions.h"

// Forward declaration
class ControllerCollection;

class Controller {
public:
    virtual ~Controller() = default;

    // Entry point to render this screen
    virtual void render();
    virtual void init();

    // Optional input handlers
    virtual void onEncoderPress() {}
    virtual void onEncoderLongPress() {}
    virtual void onEncoderUp() {}
    virtual void onEncoderDown() {}

    lv_obj_t* screenObject() const { return screen; }
    
    // Set the controller collection
    void setControllerCollection(ControllerCollection* collection) { controllerCollection = collection; }
    
    // Helper for navigation
    void navigateTo(const std::string& controllerId, uint32_t duration = 300, TransitionDirection direction = TransitionDirection::NONE);

protected:
    lv_obj_t* screen = nullptr;
    ControllerCollection* controllerCollection = nullptr;
};

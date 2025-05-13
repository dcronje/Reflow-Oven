#pragma once

#include <memory>
#include <string>
#include "types/transitions.h"
#include "lvgl.h"

// Forward declarations
class Controller;
class ControllerCollectionImpl;

class ControllerCollection {
public:
    ControllerCollection();
    ~ControllerCollection();

    void init(lv_obj_t* parent, int x, int y, int width, int height);
    void registerController(const std::string& id, Controller* controller);
    void navigateTo(const std::string& id, uint32_t duration = 300, TransitionDirection direction = TransitionDirection::NONE);
    Controller* currentController() const;
    void update();

    // Input forwarding
    void handleEncoderUp();
    void handleEncoderDown();
    void handleEncoderPress();
    void handleEncoderLongPress();

private:
    std::unique_ptr<ControllerCollectionImpl> impl;
};

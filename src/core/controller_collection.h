#pragma once

#include <memory>
#include <string>
#include "types/transitions.h"
#include "lvgl.h"
#include "core/input_events_interface.h"

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

    void update(); // optional

    // Encoder event handlers
    void handleEncoderUp();
    void handleEncoderDown();
    
    // For compatibility with existing code
    void handleEncoderPress();  // Maps to handleButtonPress(ENCODER_BUTTON)
    void handleEncoderLongPress(); // Maps to handleButtonLongPress(ENCODER_BUTTON)

    // Button event handlers
    void handleButtonPress(int buttonId);
    void handleButtonLongPress(int buttonId);

    void markDirty();
    bool isDirty() const;
    void invalidateActiveController();

private:
    std::unique_ptr<ControllerCollectionImpl> impl;
};

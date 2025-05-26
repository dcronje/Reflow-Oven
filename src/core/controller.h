#pragma once

#include <string>
#include "lvgl.h"
#include "types/transitions.h"

// Forward declarations
class ControllerCollection;
class ControllerCollectionImpl;

class Controller {
public:
    enum class State {
        UNINITIALIZED,
        ACTIVE,
        TEARDOWN
    };

protected:
    lv_obj_t* rootView = nullptr;
    ControllerCollection* controllerCollection = nullptr;
    State state = State::UNINITIALIZED;

public:
    virtual ~Controller() = default;

    // Only ControllerCollection and its implementation can set the state
    friend class ControllerCollection;
    friend class ControllerCollectionImpl;
    void setState(State newState) { 
        printf("[Controller] State transition: %s -> %s\n", 
            state == State::UNINITIALIZED ? "UNINITIALIZED" :
            state == State::ACTIVE ? "ACTIVE" : "TEARDOWN",
            newState == State::UNINITIALIZED ? "UNINITIALIZED" :
            newState == State::ACTIVE ? "ACTIVE" : "TEARDOWN");
        state = newState; 
    }

    void render(lv_obj_t* parent);

    // Lifecycle methods
    virtual void viewDidLoad() {}  // Called after view is created and attached
    virtual void viewWillAppear() {}  // Called before view becomes visible
    virtual void viewDidAppear() {}   // Called after view becomes visible
    virtual void viewWillDisappear() {}  // Called before view is hidden
    virtual void viewDidDisappear() {}   // Called after view is hidden
    virtual void viewWillUnload() {}     // Called before view is destroyed

    // Responsible for creating the view if needed
    virtual void buildView(lv_obj_t* parent) = 0;

    // Return the root LVGL object of this controller
    lv_obj_t* getView() const { return rootView; }

    // Safe navigation method
    void navigateToSafe(const std::string& controllerId, uint32_t duration = 300, 
                       TransitionDirection direction = TransitionDirection::SLIDE_IN_LEFT);

    // Optional input event handlers
    virtual void onEncoderUp() {}
    virtual void onEncoderDown() {}
    virtual void onEncoderPress() {}
    virtual void onEncoderLongPress() {}

    // Button event handlers
    virtual void onButton1Press() {}
    virtual void onButton1LongPress() {}
    virtual void onButton2Press() {}
    virtual void onButton2LongPress() {}
    virtual void onButton3Press() {}
    virtual void onButton3LongPress() {}
    virtual void onButton4Press() {}
    virtual void onButton4LongPress() {}

    // Called once at startup
    virtual void init() {}

    void setControllerCollection(ControllerCollection* collection) {
        controllerCollection = collection;
    }

    void navigateTo(const std::string& controllerId, uint32_t duration = 300, 
                   TransitionDirection direction = TransitionDirection::SLIDE_IN_LEFT);
    void invalidateView();

    State getState() const { return state; }
};

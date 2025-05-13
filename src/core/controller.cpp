#include "controller.h"
#include "controller_collection.h"

void Controller::render() {
    if (screen) {
        lv_obj_clean(screen);
    } else {
        screen = lv_obj_create(NULL);
        lv_scr_load(screen);
    }
    // Derived classes should call their view render functions here
}

void Controller::init() {
    // Default implementation does nothing
}

void Controller::navigateTo(const std::string& controllerId, uint32_t duration, TransitionDirection direction) {
    if (controllerCollection) {
        controllerCollection->navigateTo(controllerId, duration, direction);
    }
}


// controller_collection.cpp
#include "controller_collection.h"
#include "controller.h"
#include "lvgl.h"
#include <map>

// Forward declarations for lambda functions
static void setXPosition(void* obj, int32_t v);
static void setOpacity(void* obj, int32_t v);

// Lambda function implementations
static void setXPosition(void* obj, int32_t v) {
    lv_obj_set_x(static_cast<lv_obj_t*>(obj), v);
}

static void setOpacity(void* obj, int32_t v) {
    lv_obj_set_style_opa(static_cast<lv_obj_t*>(obj), v, 0);
}

class ControllerCollectionImpl {
public:
    std::map<std::string, Controller*> controllers;
    Controller* activeController = nullptr;
    lv_obj_t* container = nullptr;

    void animateTransition(Controller* from, Controller* to, uint32_t duration, TransitionDirection direction) {
        if (!to || !container) return;

        lv_obj_t* oldScreen = from ? from->screenObject() : nullptr;

        // Create and render the new controller view
        to->render();
        lv_obj_t* newScreen = to->screenObject();
        lv_obj_set_parent(newScreen, container);

        if (direction == TransitionDirection::NONE || !oldScreen) {
            lv_obj_move_foreground(newScreen);
            return;
        }

        // Animate transition based on direction
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_time(&a, duration);

        switch (direction) {
            case TransitionDirection::SLIDE_IN_LEFT:
                lv_obj_set_x(newScreen, -lv_obj_get_width(container));
                lv_anim_set_var(&a, newScreen);
                lv_anim_set_exec_cb(&a, setXPosition);
                lv_anim_set_values(&a, -lv_obj_get_width(container), 0);
                break;
            case TransitionDirection::SLIDE_OUT_LEFT:
                if (oldScreen) lv_obj_set_x(oldScreen, 0);
                lv_anim_set_var(&a, oldScreen);
                lv_anim_set_exec_cb(&a, setXPosition);
                lv_anim_set_values(&a, 0, -lv_obj_get_width(container));
                break;
            case TransitionDirection::FADE:
                lv_obj_set_style_opa(newScreen, LV_OPA_TRANSP, 0);
                lv_anim_set_var(&a, newScreen);
                lv_anim_set_exec_cb(&a, setOpacity);
                lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
                break;
            default:
                break;
        }

        lv_anim_start(&a);
        lv_obj_move_foreground(newScreen);
    }
};

ControllerCollection::ControllerCollection() : impl(new ControllerCollectionImpl()) {
}

ControllerCollection::~ControllerCollection() = default;

void ControllerCollection::init(lv_obj_t* parent, int x, int y, int width, int height) {
    impl->container = lv_obj_create(parent);
    lv_obj_remove_style_all(impl->container);
    lv_obj_set_pos(impl->container, x, y);
    lv_obj_set_size(impl->container, width, height);
    lv_obj_set_scrollbar_mode(impl->container, LV_SCROLLBAR_MODE_OFF);
}

void ControllerCollection::registerController(const std::string& id, Controller* controller) {
    impl->controllers[id] = controller;
    controller->setControllerCollection(this);
}

void ControllerCollection::navigateTo(const std::string& id, uint32_t duration, TransitionDirection direction) {
    auto it = impl->controllers.find(id);
    if (it == impl->controllers.end()) return;

    Controller* next = it->second;
    if (impl->activeController == next) return;

    impl->animateTransition(impl->activeController, next, duration, direction);
    impl->activeController = next;
}

Controller* ControllerCollection::currentController() const {
    return impl->activeController;
}

void ControllerCollection::update() {
    if (impl->activeController) impl->activeController->render();
}

void ControllerCollection::handleEncoderUp() {
    if (impl->activeController) impl->activeController->onEncoderUp();
}

void ControllerCollection::handleEncoderDown() {
    if (impl->activeController) impl->activeController->onEncoderDown();
}

void ControllerCollection::handleEncoderPress() {
    if (impl->activeController) impl->activeController->onEncoderPress();
}

void ControllerCollection::handleEncoderLongPress() {
    if (impl->activeController) impl->activeController->onEncoderLongPress();
}

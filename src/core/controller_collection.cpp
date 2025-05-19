#include "controller_collection.h"
#include "controller.h"
#include <map>

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
    bool dirty = false;

    void animateTransition(Controller* from, Controller* to, uint32_t duration, TransitionDirection direction) {
        if (!to || !container) return;

        lv_obj_t* oldView = from ? from->getView() : nullptr;

        // If no transition or nothing to animate, just replace
        if (direction == TransitionDirection::NONE || !oldView) {
            if (from) from->willUnload();
            lv_obj_clean(container);
            to->render(container);
            to->didAppear();
            return;
        }

        // Render new view off-screen for animation
        to->render(container);
        lv_obj_t* newView = to->getView();

        switch (direction) {
            case TransitionDirection::SLIDE_IN_LEFT:
                lv_obj_set_x(newView, -lv_obj_get_width(container));
                break;
            case TransitionDirection::SLIDE_OUT_LEFT:
                lv_obj_set_x(newView, 0); // animate old out, new stays
                break;
            case TransitionDirection::FADE:
                lv_obj_set_style_opa(newView, LV_OPA_TRANSP, 0);
                break;
            default:
                break;
        }

        // Setup animation
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_time(&anim, duration);

        if (direction == TransitionDirection::SLIDE_IN_LEFT) {
            lv_anim_set_var(&anim, newView);
            lv_anim_set_exec_cb(&anim, setXPosition);
            lv_anim_set_values(&anim, -lv_obj_get_width(container), 0);
        } else if (direction == TransitionDirection::SLIDE_OUT_LEFT && oldView) {
            lv_anim_set_var(&anim, oldView);
            lv_anim_set_exec_cb(&anim, setXPosition);
            lv_anim_set_values(&anim, 0, -lv_obj_get_width(container));
        } else if (direction == TransitionDirection::FADE) {
            lv_anim_set_var(&anim, newView);
            lv_anim_set_exec_cb(&anim, setOpacity);
            lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
        }

        lv_anim_start(&anim);

        if (from) from->willUnload();
        to->didAppear();
    }
};

ControllerCollection::ControllerCollection() : impl(new ControllerCollectionImpl()) {}
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

void ControllerCollection::handleEncoderUp() {
    if (impl->activeController) impl->activeController->onEncoderUp();
}

void ControllerCollection::handleEncoderDown() {
    if (impl->activeController) impl->activeController->onEncoderDown();
}

void ControllerCollection::handleEncoderPress() {
    if (impl->activeController) {
        // Convert encoder press to button press with ENCODER_BUTTON ID
        impl->activeController->onButtonPress(ButtonId::ENCODER_BUTTON);
    }
}

void ControllerCollection::handleEncoderLongPress() {
    if (impl->activeController) {
        // Convert encoder long press to button long press with ENCODER_BUTTON ID
        impl->activeController->onButtonLongPress(ButtonId::ENCODER_BUTTON);
    }
}

void ControllerCollection::handleButtonPress(int buttonId) {
    if (impl->activeController) impl->activeController->onButtonPress(buttonId);
}

void ControllerCollection::handleButtonLongPress(int buttonId) {
    if (impl->activeController) impl->activeController->onButtonLongPress(buttonId);
}

void ControllerCollection::markDirty() {
    impl->dirty = true;
}

bool ControllerCollection::isDirty() const {
    return impl->dirty;
}

void ControllerCollection::invalidateActiveController() {
    if (impl->activeController && impl->container) {
        lv_obj_clean(impl->container);
        impl->activeController->render(impl->container);
        impl->dirty = false;
    }
}

void ControllerCollection::update() {
    // Optional future hook
}

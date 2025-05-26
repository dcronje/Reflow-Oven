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
        if (from) {
            from->setState(Controller::State::TEARDOWN);
            from->viewWillUnload();
        }

        if (!to || !container) {
            printf("ERROR: Invalid transition - to: %p, container: %p\n", (void*)to, (void*)container);
            return;
        }

        // Simple direct view swap without animation
        lv_obj_clean(container);
        to->render(container);
        
        lv_obj_t* newView = to->getView();
        if (!newView) {
            printf("ERROR: New view is null after render\n");
            return;
        }
        
        // Verify the view is properly attached to container
        lv_obj_t* parent = lv_obj_get_parent(newView);
        if (parent != container) {
            printf("ERROR: View parent mismatch - view: %p, container: %p\n", (void*)parent, (void*)container);
        }
        
        if (to) {
            to->setState(Controller::State::ACTIVE);
            to->viewDidAppear();
        }
    }
};

ControllerCollection::ControllerCollection() : impl(new ControllerCollectionImpl()) {}
ControllerCollection::~ControllerCollection() = default;

void ControllerCollection::init(lv_obj_t* parent, int x, int y, int width, int height) {
    impl->container = lv_obj_create(parent);
    if (!impl->container) {
        printf("ERROR: Failed to create container\n");
        return;
    }
    
    lv_obj_remove_style_all(impl->container);
    lv_obj_set_pos(impl->container, x, y);
    lv_obj_set_size(impl->container, width, height);
    lv_obj_set_scrollbar_mode(impl->container, LV_SCROLLBAR_MODE_OFF);
    
    // Verify container is properly attached to parent
    lv_obj_t* containerParent = lv_obj_get_parent(impl->container);
    if (containerParent != parent) {
        printf("ERROR: Container parent mismatch - container: %p, parent: %p\n", (void*)containerParent, (void*)parent);
    }
}

void ControllerCollection::registerController(const std::string& id, Controller* controller) {
    impl->controllers[id] = controller;
    controller->setControllerCollection(this);
}

void ControllerCollection::navigateTo(const std::string& id, uint32_t duration, TransitionDirection direction) {
    auto it = impl->controllers.find(id);
    if (it == impl->controllers.end()) {
        printf("ERROR: Controller '%s' not found\n", id.c_str());
        return;
    }

    Controller* next = it->second;
    if (impl->activeController == next) {
        return; // Already on this controller
    }

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
    if (impl->activeController) impl->activeController->onEncoderPress();
}

void ControllerCollection::handleEncoderLongPress() {
    if (impl->activeController) impl->activeController->onEncoderLongPress();
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

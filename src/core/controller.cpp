#include "controller.h"
#include "controller_collection.h"

void Controller::navigateTo(const std::string& controllerId, uint32_t duration, TransitionDirection direction) {
    if (controllerCollection) {
        controllerCollection->navigateTo(controllerId, duration, direction);
    }
}

void Controller::invalidateView() {
    if (controllerCollection) {
        controllerCollection->markDirty();
    }
}

void Controller::render(lv_obj_t* parent) {
    if (rootView && lv_obj_is_valid(rootView)) {
        lv_obj_del(rootView);
    }

    rootView = lv_obj_create(parent);
    lv_obj_remove_style_all(rootView);
    lv_obj_set_size(rootView, lv_obj_get_width(parent), lv_obj_get_height(parent));

    buildView(rootView); // <-- subclasses must implement this
}

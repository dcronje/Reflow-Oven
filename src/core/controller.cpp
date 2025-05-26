#include "controller.h"
#include "controller_collection.h"
#include <tuple>

void Controller::navigateToSafe(const std::string& controllerId, uint32_t duration, TransitionDirection direction) {
    if (state == State::TEARDOWN) {
        printf("WARNING: Navigation attempted during teardown\n");
        return;
    }

    // Create a timer to handle navigation in the LVGL context
    auto* params = new std::tuple<ControllerCollection*, std::string, uint32_t, TransitionDirection>(
        controllerCollection, controllerId, duration, direction);
    
    lv_timer_create([](lv_timer_t* timer) {
        auto* params = static_cast<std::tuple<ControllerCollection*, std::string, uint32_t, TransitionDirection>*>(
            lv_timer_get_user_data(timer));
        auto [collection, id, duration, dir] = *params;
        
        if (collection) {
            collection->navigateTo(id, duration, dir);
        }
        
        delete params;
        lv_timer_del(timer);
    }, 10, params);
}

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
    if (!parent || !lv_obj_is_valid(parent)) {
        printf("ERROR: Invalid parent in render\n");
        return;
    }
    
    // Clean up existing view if it exists
    if (rootView) {
        if (lv_obj_is_valid(rootView)) {
            lv_obj_del(rootView);
        }
        rootView = nullptr;
    }

    // Create new root view
    rootView = lv_obj_create(parent);
    if (!rootView) {
        printf("ERROR: Failed to create root view\n");
        return;
    }
    
    // Verify root view is properly attached to parent
    lv_obj_t* viewParent = lv_obj_get_parent(rootView);
    if (viewParent != parent) {
        printf("ERROR: View parent mismatch - view: %p, parent: %p\n", (void*)viewParent, (void*)parent);
        return;
    }
    
    // Set up root view
    lv_obj_remove_style_all(rootView);
    int parentWidth = lv_obj_get_width(parent);
    int parentHeight = lv_obj_get_height(parent);
    lv_obj_set_size(rootView, parentWidth, parentHeight);
    
    // Build the view hierarchy
    buildView(rootView);
    
    // Verify final view state
    if (!lv_obj_is_valid(rootView)) {
        printf("ERROR: Root view invalid after build\n");
        return;
    }
}

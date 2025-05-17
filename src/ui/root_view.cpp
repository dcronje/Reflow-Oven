// root_view.cpp
#include "root_view.h"
#include "lvgl.h"
#include "controllers/main_menu_controller.h"
// #include "controllers/calibration_controller.h"
// #include "controllers/reflow_controller.h"

RootView::RootView() : rootScreen(nullptr), topBar(nullptr), sideBar(nullptr) {}

// void RootView::init(lv_display_t* display) {
//     lv_obj_t* label = lv_label_create(lv_scr_act());
//     lv_label_set_text(label, "Display OK");
//     lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
// }

void RootView::init(lv_display_t* display) {
    // Create root screen associated with the provided display
    lv_display_set_default(display);
    rootScreen = lv_obj_create(nullptr);  // this becomes a new screen root
    lv_scr_load(rootScreen);              // this screen is now visible

    // Initialize controller collection with the content area as the parent
    controllerCollection = std::make_unique<ControllerCollection>();
    controllerCollection->init(rootScreen, 0, 0, lv_obj_get_width(rootScreen), lv_obj_get_height(rootScreen));
    
    // Register controllers
    controllerCollection->registerController("home", &MainMenuController::getInstance());
    // controllerCollection->registerController("calibration", &CalibrationController::getInstance());
    // controllerCollection->registerController("reflow", &ReflowController::getInstance());
    // Add other controllers that may be missing
    // Note: There is a reference to "profile-selection" in MainMenuController that may need its controller registered
    // controllerCollection->registerController("profile-selection", &ProfileSelectionController::getInstance());
    // controllerCollection->registerController("settings", &SettingsController::getInstance());
    
    // Start with the home screen
    controllerCollection->navigateTo("home");
}

void RootView::update() {
    // This is called when something in the UI needs to be updated
    if (controllerCollection) {
        // We simply forward the invalidation to the active controller
        controllerCollection->invalidateActiveController();
    }
}

// Keep other button handlers
void RootView::handleTopButtonPress() {
    // Implementation goes here
}

void RootView::handleTopButtonLongPress() {
    // Implementation goes here
}

void RootView::handleBottomButtonPress() {
    // Implementation goes here
}

void RootView::handleBottomButtonLongPress() {
    // Implementation goes here
}

// Add encoder event handlers
void RootView::handleEncoderUp() {
    if (controllerCollection) {
        controllerCollection->handleEncoderUp();
    }
}

void RootView::handleEncoderDown() {
    if (controllerCollection) {
        controllerCollection->handleEncoderDown();
    }
}

void RootView::handleEncoderPress() {
    if (controllerCollection) {
        controllerCollection->handleEncoderPress();
    }
}

void RootView::handleEncoderLongPress() {
    if (controllerCollection) {
        controllerCollection->handleEncoderLongPress();
    }
}





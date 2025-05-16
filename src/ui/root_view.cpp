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
    // rootScreen = lv_obj_create(lv_scr_act());
    // lv_display_set_default(display); // Set this display as default
    // lv_scr_load(rootScreen);


    lv_display_set_default(display);
    rootScreen = lv_obj_create(nullptr);  // this becomes a new screen root
    lv_scr_load(rootScreen);              // this screen is now visible

    // // Create the top bar
    // topBar = lv_obj_create(rootScreen);
    // lv_obj_set_size(topBar, LV_HOR_RES, 20);
    // lv_obj_set_style_bg_color(topBar, lv_color_hex(0x222222), 0);
    // lv_obj_align(topBar, LV_ALIGN_TOP_MID, 0, 0);

    // // Create the right sidebar
    // sideBar = lv_obj_create(rootScreen);
    // lv_obj_set_size(sideBar, 30, LV_VER_RES - 20);
    // lv_obj_set_style_bg_color(sideBar, lv_color_hex(0x444444), 0);
    // lv_obj_align_to(sideBar, topBar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);

    // // Create the content area where controllers will be rendered
    // contentArea = lv_obj_create(rootScreen);
    // lv_obj_set_size(contentArea, LV_HOR_RES - 30, LV_VER_RES - 20);
    // lv_obj_align(contentArea, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    // lv_obj_set_style_bg_color(contentArea, lv_color_hex(0x202020), 0);
    // lv_obj_clear_flag(contentArea, LV_OBJ_FLAG_SCROLLABLE);

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
    // It's not called in a loop, but rather when a specific event occurs
    if (controllerCollection) {
        // We simply forward the invalidation to the active controller
        controllerCollection->invalidateActiveController();
    }
}

void RootView::scheduleEncoderDownHandler(uint32_t delayMs) {
    lv_timer_create(encoderDownCallback, delayMs, this);
}

void RootView::encoderDownCallback(struct _lv_timer_t* timer) {
    RootView* view = static_cast<RootView*>(lv_timer_get_user_data(timer));
    if (view && view->controllerCollection) view->controllerCollection->handleEncoderDown();
    lv_timer_del(timer);
}

void RootView::scheduleEncoderUpHandler(uint32_t delayMs) {
    lv_timer_create(encoderUpCallback, delayMs, this);
}

void RootView::encoderUpCallback(struct _lv_timer_t* timer) {
    RootView* view = static_cast<RootView*>(lv_timer_get_user_data(timer));
    if (view && view->controllerCollection) view->controllerCollection->handleEncoderUp();
    lv_timer_del(timer);
}

void RootView::scheduleEncoderPressHandler(uint32_t delayMs) {
    lv_timer_create(encoderPressCallback, delayMs, this);
}

void RootView::encoderPressCallback(struct _lv_timer_t* timer) {
    RootView* view = static_cast<RootView*>(lv_timer_get_user_data(timer));
    if (view && view->controllerCollection) view->controllerCollection->handleEncoderPress();
    lv_timer_del(timer);
}

void RootView::scheduleEncoderLongPressHandler(uint32_t delayMs) {
    lv_timer_create(encoderLongPressCallback, delayMs, this);
}

void RootView::encoderLongPressCallback(struct _lv_timer_t* timer) {
    RootView* view = static_cast<RootView*>(lv_timer_get_user_data(timer));
    if (view && view->controllerCollection) view->controllerCollection->handleEncoderLongPress();
    lv_timer_del(timer);
}





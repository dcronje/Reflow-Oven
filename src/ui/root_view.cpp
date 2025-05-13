// root_view.cpp
#include "root_view.h"
#include "lvgl.h"
#include "controllers/main_menu_controller.h"
#include "controllers/calibration_controller.h"
#include "controllers/reflow_controller.h"

RootView::RootView() : rootScreen(nullptr), topBar(nullptr), sideBar(nullptr) {}

void RootView::init(lv_display_t* display) {
    rootScreen = lv_obj_create(nullptr);
    lv_scr_load(rootScreen);

    // Create the top bar
    topBar = lv_obj_create(rootScreen);
    lv_obj_set_size(topBar, LV_HOR_RES, 20);
    lv_obj_set_style_bg_color(topBar, lv_color_hex(0x222222), 0);
    lv_obj_align(topBar, LV_ALIGN_TOP_MID, 0, 0);

    // Create the right sidebar
    sideBar = lv_obj_create(rootScreen);
    lv_obj_set_size(sideBar, 30, LV_VER_RES - 20);
    lv_obj_set_style_bg_color(sideBar, lv_color_hex(0x444444), 0);
    lv_obj_align_to(sideBar, topBar, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);

    // Initialize controller collection inside the content area
    controllerCollection = std::make_unique<ControllerCollection>();
    controllerCollection->init(rootScreen, 0, 20, LV_HOR_RES - 30, LV_VER_RES - 20);
    
    // Register controllers
    controllerCollection->registerController("home", &MainMenuController::getInstance());
    controllerCollection->registerController("calibration", &CalibrationController::getInstance());
    controllerCollection->registerController("reflow", &ReflowController::getInstance());
    
    // Start with the home screen
    controllerCollection->navigateTo("home");
}

void RootView::update() {
    if (controllerCollection) controllerCollection->update();
}

void RootView::handleEncoderUp() {
    if (controllerCollection) controllerCollection->handleEncoderUp();
}
void RootView::handleEncoderDown() {
    if (controllerCollection) controllerCollection->handleEncoderDown();
}
void RootView::handleEncoderPress() {
    if (controllerCollection) controllerCollection->handleEncoderPress();
}
void RootView::handleEncoderLongPress() {
    if (controllerCollection) controllerCollection->handleEncoderLongPress();
}

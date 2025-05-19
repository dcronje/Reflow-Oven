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

// Encoder event handlers
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

// Button event handlers
void RootView::handleButtonPress(int buttonId) {
    // Map button IDs to specific actions or forward to controller collection
    if (controllerCollection) {
        switch (buttonId) {
            case ButtonId::ENCODER_BUTTON:
                // Encoder button action (generally confirm/select)
                printf("Encoder button pressed\n");
                controllerCollection->handleButtonPress(buttonId);
                break;
                
            case ButtonId::BUTTON_1:
                // Button 1 action (could be "back" or "menu")
                printf("Button 1 pressed\n");
                controllerCollection->handleButtonPress(buttonId);
                break;
                
            case ButtonId::BUTTON_2:
                // Button 2 action (could be "ok" or "select")
                printf("Button 2 pressed\n");
                controllerCollection->handleButtonPress(buttonId);
                break;
                
            case ButtonId::BUTTON_3:
                // Button 3 action (left/previous)
                printf("Button 3 pressed\n");
                // Optionally map to encoder up for navigation
                controllerCollection->handleEncoderUp();
                break;
                
            case ButtonId::BUTTON_4:
                // Button 4 action (right/next)
                printf("Button 4 pressed\n");
                // Optionally map to encoder down for navigation
                controllerCollection->handleEncoderDown();
                break;
                
            default:
                printf("Unknown button pressed: %d\n", buttonId);
                controllerCollection->handleButtonPress(buttonId);
                break;
        }
    }
}

void RootView::handleButtonLongPress(int buttonId) {
    // Map button IDs to specific actions or forward to controller collection
    if (controllerCollection) {
        switch (buttonId) {
            case ButtonId::ENCODER_BUTTON:
                // Encoder button long press (could be "cancel" or "back")
                printf("Encoder button long pressed\n");
                controllerCollection->handleButtonLongPress(buttonId);
                break;
                
            case ButtonId::BUTTON_1:
                // Button 1 long press action (could be "home" or "main menu")
                printf("Button 1 long pressed\n");
                controllerCollection->navigateTo("home"); // Always return to home
                break;
                
            case ButtonId::BUTTON_2:
                // Button 2 long press action
                printf("Button 2 long pressed\n");
                controllerCollection->handleButtonLongPress(buttonId);
                break;
                
            case ButtonId::BUTTON_3:
                // Button 3 long press action
                printf("Button 3 long pressed\n");
                controllerCollection->handleButtonLongPress(buttonId);
                break;
                
            case ButtonId::BUTTON_4:
                // Button 4 long press action
                printf("Button 4 long pressed\n");
                controllerCollection->handleButtonLongPress(buttonId);
                break;
                
            default:
                printf("Unknown button long pressed: %d\n", buttonId);
                controllerCollection->handleButtonLongPress(buttonId);
                break;
        }
    }
}





#pragma once

#include <vector>
#include "core/controller.h"
#include "lvgl.h"
#include "core/event_subscriber.h"

class MainMenuController : public Controller {
public:
    static MainMenuController& getInstance();

    void buildView(lv_obj_t* parent) override;
    void init() override;
    void willUnload() override;

    void onEncoderPress() override;
    void onEncoderUp() override;
    void onEncoderDown() override;
    void onEncoderLongPress() override;

    // UI action methods
    void selectReflowCurve();
    void startReflow();
    void calibrate();
    void openSettings();
    void toggleDoor();
    
    // Method to refresh the door status button - can be called from the main thread
    void refreshDoorStatusButton();

private:
    MainMenuController() = default;
    ~MainMenuController();

    lv_obj_t* menu = nullptr;
    std::vector<lv_obj_t*> buttons;
    int selectedIndex = 0;
    lv_timer_t* updateTimer = nullptr; // Timer for periodic UI updates
    
    // Event handling
    EventSubscriber eventSubscriber;
    TaskHandle_t eventTaskHandle = nullptr;
    
    void updateButtonFocus(bool animated = true);
    
    // Event processing task
    static void eventProcessingTask(void* pvParameters);
    
    // Timer callback for UI updates
    static void updateTimerCallback(lv_timer_t* timer);
    void periodicUpdate();
};

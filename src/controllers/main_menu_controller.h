#pragma once

#include <vector>
#include "core/controller.h"
#include "lvgl.h"
#include "core/event_subscriber.h"

class MainMenuController : public Controller {
public:
    static MainMenuController& getInstance();

    void buildView(lv_obj_t* parent) override;
    void viewDidLoad() override;
    void viewWillAppear() override;
    void viewDidAppear() override;
    void viewWillDisappear() override;
    void viewDidDisappear() override;
    void viewWillUnload() override;
    void init() override;

    void onEncoderPress() override;
    void onEncoderUp() override;
    void onEncoderDown() override;
    void onEncoderLongPress() override;

    // UI action methods
    void viewCurves();
    void calibrateSensors();
    void calibrateDoor();
    void calibrateOven();
    void openSettings();
    void returnToHome();

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
    
    void startEventTask();
    void stopEventTask();
    void startUpdateTimer();
    void stopUpdateTimer();
    
    // Event processing task
    static void eventProcessingTask(void* pvParameters);
    
    // Timer callback for UI updates
    static void updateTimerCallback(lv_timer_t* timer);
    void periodicUpdate();
};

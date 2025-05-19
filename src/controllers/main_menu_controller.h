#pragma once

#include <vector>
#include "core/controller.h"
#include "lvgl.h"
#include "core/message_handler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class MainMenuController : public Controller, public MessageHandler {
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

    // MessageHandler interface implementation
    bool processMessage(const void* data, size_t size) override;
    bool processMessage(const std::string& serialized) override;

private:
    MainMenuController() = default;
    ~MainMenuController();

    lv_obj_t* menu = nullptr;
    std::vector<lv_obj_t*> buttons;
    int selectedIndex = 0;
    lv_timer_t* updateTimer = nullptr; // Timer for periodic UI updates
    
    // Message handling
    QueueHandle_t messageQueue = nullptr;
    TaskHandle_t messageTaskHandle = nullptr;
    
    void updateButtonFocus(bool animated = true);
    
    // Message processing task
    static void messageProcessingTask(void* pvParameters);
    
    // Timer callback for UI updates
    static void updateTimerCallback(lv_timer_t* timer);
    void periodicUpdate();

    // Process specific message types
    bool processDoorMessage(const void* data, size_t size);
    bool processSystemMessage(const void* data, size_t size);
};

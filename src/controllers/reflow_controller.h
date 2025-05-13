#pragma once

#include "core/controller.h"
#include "models/reflow_model.h"
#include <lvgl.h>
#include "FreeRTOS.h"
#include "task.h"

enum class ReflowState {
    PRECHECK,
    RUNNING,
    COMPLETE
};

class ReflowController : public Controller {
public:
    static ReflowController& getInstance();

    // Controller interface methods
    void render() override;
    void init() override;
    void onEncoderPress() override;
    void onEncoderUp() override;
    void onEncoderDown() override;
    void onEncoderLongPress() override;

    // Navigation methods
    void returnToMainMenu();

    // Reflow control methods
    void requestStart();          // Start precheck
    void confirmStart();          // Confirm and start reflow process
    void cancel();                // Cancel the reflow process

    // Model access
    ReflowModel& getModel();
    const ReflowState& getState() const;
    float getElapsedMsInStep() const;
    float getCurrentTargetTemp() const;

    // Temperature management
    void setCurrentTemp(float tempC);
    float getCurrentTemp() const;

private:
    ReflowController();
    ~ReflowController();

    // UI elements
    lv_obj_t* precheckScreen = nullptr;  // Precheck screen UI
    lv_obj_t* processScreen = nullptr;   // Running process UI
    lv_obj_t* summaryScreen = nullptr;   // Completed process UI
    lv_obj_t* currentContainer = nullptr; // Current active container
    
    // Chart for temperature display
    lv_obj_t* tempChart = nullptr;
    lv_chart_series_t* tempSeries = nullptr;
    lv_chart_series_t* targetSeries = nullptr;
    
    // Status labels
    lv_obj_t* statusLabel = nullptr;
    lv_obj_t* tempLabel = nullptr;
    lv_obj_t* timeLabel = nullptr;
    lv_obj_t* stepLabel = nullptr;
    
    // Button handling
    int selectedButton = 0;
    bool confirmButtonActive = false;
    
    // Task for temperature control
    TaskHandle_t reflowTask = nullptr;
    static void reflowTaskFunc(void* param);
    void runReflowProcess();
    void updateReflowUI();
    
    // Helper functions
    void renderPrecheckScreen();
    void renderProcessScreen();
    void renderSummaryScreen();
    void switchToScreen(lv_obj_t* screen);
    void updateButtonFocus();
    
    // Timer callback for UI updates
    static void updateTimerCallback(lv_timer_t* timer);
    lv_timer_t* updateTimer = nullptr;

    // Model and state
    ReflowModel model;
    ReflowState state = ReflowState::PRECHECK;

    bool reflowRequested = false;
    bool coolingRequired = false;
    float currentTemp = 25.0f;
    uint32_t stepStartTimeMs = 0;
};

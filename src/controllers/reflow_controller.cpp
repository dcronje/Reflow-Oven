#include "controllers/reflow_controller.h"
#include "services/buzzer_service.h"
#include "services/temperature_control_service.h"
#include "pico/time.h"
#include <cmath>
#include <algorithm>
#include "lvgl.h"

ReflowController& ReflowController::getInstance() {
    static ReflowController instance;
    return instance;
}

ReflowController::ReflowController() {
    // Set up default reflow curve on creation
    model.setActiveCurve({
        "LEAD-FREE",
        50.0f,
        {
            {"Preheat", 150, 60000},
            {"Soak", 180, 40000},
            {"Reflow", 245, 30000},
            {"Cool", 50, 60000}
        }
    });
}

ReflowController::~ReflowController() {
    if (updateTimer) {
        lv_timer_del(updateTimer);
        updateTimer = nullptr;
    }
    
    if (reflowTask) {
        vTaskDelete(reflowTask);
        reflowTask = nullptr;
    }
}

void ReflowController::init() {
    // Initialize controller resources
}

void ReflowController::render() {
    Controller::render(); // Call parent render to initialize the screen
    
    // Create UI based on current state
    switch (state) {
        case ReflowState::PRECHECK:
            renderPrecheckScreen();
            break;
        case ReflowState::RUNNING:
            renderProcessScreen();
            break;
        case ReflowState::COMPLETE:
            renderSummaryScreen();
            break;
    }
    
    // Create timer for updating UI if not already created
    if (!updateTimer) {
        updateTimer = lv_timer_create(updateTimerCallback, 250, this);
    }
}

void ReflowController::renderPrecheckScreen() {
    if (!precheckScreen) {
        precheckScreen = lv_obj_create(screen);
        lv_obj_set_size(precheckScreen, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(precheckScreen, lv_color_hex(0x202020), LV_PART_MAIN);
        lv_obj_set_flex_flow(precheckScreen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(precheckScreen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(precheckScreen, 20, 0);
        
        // Title
        lv_obj_t* title = lv_label_create(precheckScreen);
        lv_label_set_text(title, "Reflow Precheck");
        lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
        
        // Info
        statusLabel = lv_label_create(precheckScreen);
        lv_label_set_text(statusLabel, "Checking conditions for reflow...");
        lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFCC00), 0);
        
        // Temperature
        tempLabel = lv_label_create(precheckScreen);
        char tempText[32];
        snprintf(tempText, sizeof(tempText), "Current temperature: %.1f°C", currentTemp);
        lv_label_set_text(tempLabel, tempText);
        
        // Profile info
        lv_obj_t* profileInfo = lv_label_create(precheckScreen);
        lv_label_set_text(profileInfo, ("Profile: " + model.getActiveCurve().name).c_str());
        
        // Create buttons container
        lv_obj_t* btnContainer = lv_obj_create(precheckScreen);
        lv_obj_remove_style_all(btnContainer);
        lv_obj_set_size(btnContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(btnContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btnContainer, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Start button
        lv_obj_t* startBtn = lv_btn_create(btnContainer);
        lv_obj_set_size(startBtn, 120, 50);
        lv_obj_t* startLabel = lv_label_create(startBtn);
        lv_label_set_text(startLabel, "Start");
        lv_obj_center(startLabel);
        lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x00AA00), 0);
        
        // Cancel button
        lv_obj_t* cancelBtn = lv_btn_create(btnContainer);
        lv_obj_set_size(cancelBtn, 120, 50);
        lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
        lv_label_set_text(cancelLabel, "Cancel");
        lv_obj_center(cancelLabel);
        lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xAA0000), 0);
    }
    
    switchToScreen(precheckScreen);
    
    // Update button focus
    selectedButton = 0; // 0 = Start, 1 = Cancel
    updateButtonFocus();
}

void ReflowController::renderProcessScreen() {
    if (!processScreen) {
        processScreen = lv_obj_create(screen);
        lv_obj_set_size(processScreen, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(processScreen, lv_color_hex(0x202020), LV_PART_MAIN);
        lv_obj_set_style_pad_all(processScreen, 10, 0);
        
        // Title
        lv_obj_t* title = lv_label_create(processScreen);
        lv_label_set_text(title, "Reflow Process");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
        
        // Status label
        statusLabel = lv_label_create(processScreen);
        lv_label_set_text(statusLabel, "Running...");
        lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 30);
        
        // Step label
        stepLabel = lv_label_create(processScreen);
        lv_label_set_text(stepLabel, "Step: Preheat");
        lv_obj_align(stepLabel, LV_ALIGN_TOP_LEFT, 10, 30);
        
        // Temperature label
        tempLabel = lv_label_create(processScreen);
        lv_label_set_text(tempLabel, "Temp: 25.0°C");
        lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 10, 50);
        
        // Time label
        timeLabel = lv_label_create(processScreen);
        lv_label_set_text(timeLabel, "Time: 0s");
        lv_obj_align(timeLabel, LV_ALIGN_TOP_RIGHT, -10, 50);
        
        // Create chart
        tempChart = lv_chart_create(processScreen);
        lv_obj_set_size(tempChart, LV_HOR_RES - 40, 150);
        lv_obj_align(tempChart, LV_ALIGN_CENTER, 0, 20);
        lv_chart_set_type(tempChart, LV_CHART_TYPE_LINE);
        lv_chart_set_range(tempChart, LV_CHART_AXIS_PRIMARY_Y, 0, 300);
        
        // Set up chart properties - avoid using lv_chart_set_axis_tick if it's not available
        lv_chart_set_div_line_count(tempChart, 5, 5);
        lv_chart_set_point_count(tempChart, 60);
        
        // Add series
        tempSeries = lv_chart_add_series(tempChart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
        targetSeries = lv_chart_add_series(tempChart, lv_color_hex(0x00FF00), LV_CHART_AXIS_PRIMARY_Y);
        
        // Cancel button at bottom
        lv_obj_t* cancelBtn = lv_btn_create(processScreen);
        lv_obj_set_size(cancelBtn, 120, 40);
        lv_obj_align(cancelBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
        lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xAA0000), 0);
        
        lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
        lv_label_set_text(cancelLabel, "Cancel");
        lv_obj_center(cancelLabel);
    }
    
    switchToScreen(processScreen);
}

void ReflowController::renderSummaryScreen() {
    if (!summaryScreen) {
        summaryScreen = lv_obj_create(screen);
        lv_obj_set_size(summaryScreen, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(summaryScreen, lv_color_hex(0x202020), LV_PART_MAIN);
        lv_obj_set_flex_flow(summaryScreen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(summaryScreen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(summaryScreen, 15, 0);
        
        // Title
        lv_obj_t* title = lv_label_create(summaryScreen);
        lv_label_set_text(title, "Reflow Complete");
        lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
        
        // Status
        statusLabel = lv_label_create(summaryScreen);
        lv_label_set_text(statusLabel, "Process completed successfully!");
        lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00FF00), 0);
        
        // Profile used
        lv_obj_t* profileInfo = lv_label_create(summaryScreen);
        lv_label_set_text(profileInfo, ("Profile: " + model.getActiveCurve().name).c_str());
        
        // Max temperature
        lv_obj_t* maxTempLabel = lv_label_create(summaryScreen);
        char tempText[32];
        snprintf(tempText, sizeof(tempText), "Maximum temperature: %.1f°C", 245.0f); // TODO: Track max temp
        lv_label_set_text(maxTempLabel, tempText);
        
        // Return button
        lv_obj_t* homeBtn = lv_btn_create(summaryScreen);
        lv_obj_set_size(homeBtn, 180, 50);
        lv_obj_t* homeLabel = lv_label_create(homeBtn);
        lv_label_set_text(homeLabel, "Return to Menu");
        lv_obj_center(homeLabel);
    }
    
    switchToScreen(summaryScreen);
    selectedButton = 0;
}

void ReflowController::switchToScreen(lv_obj_t* targetScreen) {
    if (currentContainer) {
        lv_obj_add_flag(currentContainer, LV_OBJ_FLAG_HIDDEN);
    }
    
    lv_obj_clear_flag(targetScreen, LV_OBJ_FLAG_HIDDEN);
    currentContainer = targetScreen;
}

void ReflowController::updateButtonFocus() {
    if (state == ReflowState::PRECHECK) {
        // Get the button container
        lv_obj_t* btnContainer = lv_obj_get_child(precheckScreen, lv_obj_get_child_cnt(precheckScreen) - 1);
        
        // Get start and cancel buttons
        lv_obj_t* startBtn = lv_obj_get_child(btnContainer, 0);
        lv_obj_t* cancelBtn = lv_obj_get_child(btnContainer, 1);
        
        // Update focus
        if (selectedButton == 0) {
            lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x00FF00), 0);
            lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xAA0000), 0);
        } else {
            lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x00AA00), 0);
            lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xFF0000), 0);
        }
        
        // Enable the start button if conditions are met
        confirmButtonActive = (currentTemp <= model.getActiveCurve().minimumStartTempC);
        
        if (!confirmButtonActive) {
            lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x666666), 0);
            lv_obj_set_style_text_color(startBtn, lv_color_hex(0xAAAAAA), 0);
        }
    }
}

void ReflowController::updateTimerCallback(lv_timer_t* timer) {
    ReflowController* controller = static_cast<ReflowController*>(lv_timer_get_user_data(timer));
    controller->updateReflowUI();
}

void ReflowController::updateReflowUI() {
    // Update UI based on current state
    switch (state) {
        case ReflowState::PRECHECK:
            if (tempLabel) {
                char tempText[32];
                snprintf(tempText, sizeof(tempText), "Current temperature: %.1f°C", currentTemp);
                lv_label_set_text(tempLabel, tempText);
            }
            
            if (statusLabel) {
                if (currentTemp <= model.getActiveCurve().minimumStartTempC) {
                    lv_label_set_text(statusLabel, "Ready to start! Press START to begin.");
                    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00FF00), 0);
                    confirmButtonActive = true;
                } else {
                    char statusText[64];
                    snprintf(statusText, sizeof(statusText), 
                             "Oven too hot (%.1f°C). Must be below %.1f°C",
                             currentTemp, model.getActiveCurve().minimumStartTempC);
                    lv_label_set_text(statusLabel, statusText);
                    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFF0000), 0);
                    confirmButtonActive = false;
                }
            }
            
            updateButtonFocus();
            break;
            
        case ReflowState::RUNNING:
            if (tempLabel) {
                char tempText[32];
                snprintf(tempText, sizeof(tempText), "Temp: %.1f°C", currentTemp);
                lv_label_set_text(tempLabel, tempText);
            }
            
            if (timeLabel) {
                char timeText[32];
                float elapsed = getElapsedMsInStep() / 1000.0f;
                snprintf(timeText, sizeof(timeText), "Time: %.1fs", elapsed);
                lv_label_set_text(timeLabel, timeText);
            }
            
            if (stepLabel) {
                char stepText[64];
                snprintf(stepText, sizeof(stepText), "Step: %s (%d/%d)", 
                         model.getCurrentStep().label.c_str(),
                         model.getCurrentStepIndex() + 1,
                         (int)model.getActiveCurve().steps.size());
                lv_label_set_text(stepLabel, stepText);
            }
            
            // Update chart
            if (tempChart && tempSeries && targetSeries) {
                lv_chart_set_next_value(tempChart, tempSeries, (int)currentTemp);
                lv_chart_set_next_value(tempChart, targetSeries, (int)getCurrentTargetTemp());
            }
            break;
            
        case ReflowState::COMPLETE:
            // Static display, no updates needed
            break;
    }
}

void ReflowController::onEncoderPress() {
    BuzzerService::getInstance().playMediumTone(300);
    
    switch (state) {
        case ReflowState::PRECHECK:
            if (selectedButton == 0 && confirmButtonActive) {
                confirmStart();
            } else if (selectedButton == 1) {
                cancel();
            }
            break;
            
        case ReflowState::RUNNING:
            // Cancel button is the only option when running
            cancel();
            break;
            
        case ReflowState::COMPLETE:
            // Return to menu is the only option when complete
            returnToMainMenu();
            break;
    }
}

void ReflowController::onEncoderUp() {
    if (state == ReflowState::PRECHECK) {
        selectedButton = 0; // Select start button
        updateButtonFocus();
    }
}

void ReflowController::onEncoderDown() {
    if (state == ReflowState::PRECHECK) {
        selectedButton = 1; // Select cancel button
        updateButtonFocus();
    }
}

void ReflowController::onEncoderLongPress() {
    // Long press always cancels and returns to menu
    if (state != ReflowState::COMPLETE) {
        cancel();
    } else {
        returnToMainMenu();
    }
}

void ReflowController::requestStart() {
    state = ReflowState::PRECHECK;
    model.resetProgress();
    render(); // Force re-render with precheck screen
}

void ReflowController::confirmStart() {
    if (!confirmButtonActive) return;
    
    BuzzerService::getInstance().playHighTone(2000);
    state = ReflowState::RUNNING;
    model.resetProgress();
    stepStartTimeMs = to_ms_since_boot(get_absolute_time());
    
    // Start the reflow process task
    xTaskCreate(reflowTaskFunc, "ReflowTask", 2048, this, 2, &reflowTask);
    
    // Switch to process screen
    render();
}

void ReflowController::cancel() {
    BuzzerService::getInstance().playLowTone(2000);
    
    // Stop temperature control
    TemperatureControlService::getInstance().stopHeating();
    
    // Stop the reflow task if it's running
    if (reflowTask) {
        vTaskDelete(reflowTask);
        reflowTask = nullptr;
    }
    
    // Return to main menu
    returnToMainMenu();
}

void ReflowController::returnToMainMenu() {
    navigateTo("home", 300, TransitionDirection::SLIDE_IN_RIGHT);
}

void ReflowController::reflowTaskFunc(void* param) {
    ReflowController* controller = static_cast<ReflowController*>(param);
    controller->runReflowProcess();
    vTaskDelete(nullptr);
}

void ReflowController::runReflowProcess() {
    // Reset step timer
    stepStartTimeMs = to_ms_since_boot(get_absolute_time());
    
    while (!model.isComplete()) {
        // Get current step parameters
        const ReflowStep& step = model.getCurrentStep();
        float targetTemp = getCurrentTargetTemp();
        
        // Set heater to reach target
        TemperatureControlService::getInstance().setTargetTemperature(targetTemp);
        
        // Check if current step is complete
        float elapsed = getElapsedMsInStep();
        if (elapsed >= step.durationMs) {
            BuzzerService::getInstance().playMediumTone(1000);
            model.advanceStep();
            stepStartTimeMs = to_ms_since_boot(get_absolute_time());
        }
        
        // Delay before next update
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    
    // Process complete
    BuzzerService::getInstance().playHighTone(2000);
    state = ReflowState::COMPLETE;
    
    // Stop heating
    TemperatureControlService::getInstance().stopHeating();
    
    // Switch to summary screen
    render();
}

ReflowModel& ReflowController::getModel() {
    return model;
}

const ReflowState& ReflowController::getState() const {
    return state;
}

void ReflowController::setCurrentTemp(float tempC) {
    currentTemp = tempC;
}

float ReflowController::getCurrentTemp() const {
    return currentTemp;
}

float ReflowController::getElapsedMsInStep() const {
    return to_ms_since_boot(get_absolute_time()) - stepStartTimeMs;
}

float ReflowController::getCurrentTargetTemp() const {
    const auto& step = model.getCurrentStep();
    float elapsed = getElapsedMsInStep();
    float ratio = elapsed / step.durationMs;
    if (ratio > 1.0f) ratio = 1.0f;

    float previous = (model.getCurrentStepIndex() == 0)
                     ? currentTemp
                     : model.getActiveCurve().steps[model.getCurrentStepIndex() - 1].targetTempC;

    return previous + (step.targetTempC - previous) * ratio;
}

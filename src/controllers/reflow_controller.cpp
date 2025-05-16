// #include "controllers/reflow_controller.h"
// #include "services/buzzer_service.h"
// #include "services/temperature_control_service.h"
// #include "pico/time.h"
// #include <cmath>
// #include <algorithm>
// #include "lvgl.h"

// ReflowController& ReflowController::getInstance() {
//     static ReflowController instance;
//     return instance;
// }

// ReflowController::ReflowController() {
//     // Set up default reflow curve on creation
//     model.setActiveCurve({
//         "LEAD-FREE",
//         50.0f,
//         {
//             {"Preheat", 150, 60000},
//             {"Soak", 180, 40000},
//             {"Reflow", 245, 30000},
//             {"Cool", 50, 60000}
//         }
//     });
// }

// ReflowController::~ReflowController() {
//     if (updateTimer) {
//         lv_timer_del(updateTimer);
//         updateTimer = nullptr;
//     }
    
//     if (reflowTask) {
//         vTaskDelete(reflowTask);
//         reflowTask = nullptr;
//     }
// }

// void ReflowController::init() {
//     // Initialize controller resources
// }

// void ReflowController::render(lv_obj_t* parent) {
//     // Render UI based on current state directly to parent
//     switch (state) {
//         case ReflowState::PRECHECK:
//             renderPrecheckScreen(parent);
//             break;
//         case ReflowState::RUNNING:
//             renderProcessScreen(parent);
//             break;
//         case ReflowState::COMPLETE:
//             renderSummaryScreen(parent);
//             break;
//     }
    
//     // Create timer for updating UI if not already created
//     if (!updateTimer) {
//         updateTimer = lv_timer_create(updateTimerCallback, 250, nullptr);
//     }
// }

// void ReflowController::renderPrecheckScreen(lv_obj_t* parent) {
//     // Clean existing content
//     if (parent && lv_obj_is_valid(parent)) {
//         lv_obj_clean(parent);
//     }
    
//     // Clear the button list since we're recreating them
//     buttonsList.clear();
    
//     // Set up container
//     lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
//     lv_obj_set_style_bg_color(parent, lv_color_hex(0x202020), LV_PART_MAIN);
//     lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
//     lv_obj_set_style_pad_row(parent, 20, 0);
    
//     // Title
//     lv_obj_t* title = lv_label_create(parent);
//     lv_label_set_text(title, "Reflow Precheck");
//     lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
//     lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
//     // Info
//     statusLabel = lv_label_create(parent);
//     lv_label_set_text(statusLabel, "Checking conditions for reflow...");
//     lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFCC00), 0);
    
//     // Temperature
//     tempLabel = lv_label_create(parent);
//     char tempText[32];
//     snprintf(tempText, sizeof(tempText), "Current temperature: %.1f°C", currentTemp);
//     lv_label_set_text(tempLabel, tempText);
    
//     // Profile info
//     lv_obj_t* profileInfo = lv_label_create(parent);
//     lv_label_set_text(profileInfo, ("Profile: " + model.getActiveCurve().name).c_str());
    
//     // Create buttons container
//     lv_obj_t* btnContainer = lv_obj_create(parent);
//     lv_obj_remove_style_all(btnContainer);
//     lv_obj_set_size(btnContainer, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_flex_flow(btnContainer, LV_FLEX_FLOW_ROW);
//     lv_obj_set_flex_align(btnContainer, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
//     // Start button
//     lv_obj_t* startBtn = lv_btn_create(btnContainer);
//     lv_obj_set_size(startBtn, 120, 50);
//     lv_obj_t* startLabel = lv_label_create(startBtn);
//     lv_label_set_text(startLabel, "Start");
//     lv_obj_center(startLabel);
//     lv_obj_set_style_bg_color(startBtn, lv_color_hex(0x00AA00), 0);
//     buttonsList.push_back(startBtn);
    
//     // Cancel button
//     lv_obj_t* cancelBtn = lv_btn_create(btnContainer);
//     lv_obj_set_size(cancelBtn, 120, 50);
//     lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
//     lv_label_set_text(cancelLabel, "Cancel");
//     lv_obj_center(cancelLabel);
//     lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xAA0000), 0);
//     buttonsList.push_back(cancelBtn);
    
//     // Set initial button focus
//     selectedButton = 0; // 0 = Start, 1 = Cancel
//     updateButtonFocus();
// }

// void ReflowController::renderProcessScreen(lv_obj_t* parent) {
//     // Clean existing content
//     if (parent && lv_obj_is_valid(parent)) {
//         lv_obj_clean(parent);
//     }
    
//     // Clear the button list
//     buttonsList.clear();
    
//     // Set up container
//     lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
//     lv_obj_set_style_bg_color(parent, lv_color_hex(0x202020), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(parent, 10, 0);
    
//     // Title
//     lv_obj_t* title = lv_label_create(parent);
//     lv_label_set_text(title, "Reflow Process");
//     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
//     // Status label
//     statusLabel = lv_label_create(parent);
//     lv_label_set_text(statusLabel, "Running...");
//     lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 30);
    
//     // Step label
//     stepLabel = lv_label_create(parent);
//     lv_label_set_text(stepLabel, "Step: Preheat");
//     lv_obj_align(stepLabel, LV_ALIGN_TOP_LEFT, 10, 30);
    
//     // Temperature label
//     tempLabel = lv_label_create(parent);
//     lv_label_set_text(tempLabel, "Temp: 25.0°C");
//     lv_obj_align(tempLabel, LV_ALIGN_TOP_LEFT, 10, 50);
    
//     // Time label
//     timeLabel = lv_label_create(parent);
//     lv_label_set_text(timeLabel, "Time: 0s");
//     lv_obj_align(timeLabel, LV_ALIGN_TOP_RIGHT, -10, 50);
    
//     // Create chart
//     tempChart = lv_chart_create(parent);
//     lv_obj_set_size(tempChart, LV_HOR_RES - 40, 150);
//     lv_obj_align(tempChart, LV_ALIGN_CENTER, 0, 20);
//     lv_chart_set_type(tempChart, LV_CHART_TYPE_LINE);
//     lv_chart_set_range(tempChart, LV_CHART_AXIS_PRIMARY_Y, 0, 300);
    
//     // Set up chart properties
//     lv_chart_set_div_line_count(tempChart, 5, 5);
//     lv_chart_set_point_count(tempChart, 60);
    
//     // Add series
//     tempSeries = lv_chart_add_series(tempChart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
//     targetSeries = lv_chart_add_series(tempChart, lv_color_hex(0x00FF00), LV_CHART_AXIS_PRIMARY_Y);
    
//     // Cancel button at bottom
//     lv_obj_t* cancelBtn = lv_btn_create(parent);
//     lv_obj_set_size(cancelBtn, 120, 40);
//     lv_obj_align(cancelBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
//     lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xAA0000), 0);
    
//     lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
//     lv_label_set_text(cancelLabel, "Cancel");
//     lv_obj_center(cancelLabel);
    
//     buttonsList.push_back(cancelBtn);
//     selectedButton = 0;
//     updateButtonFocus();
// }

// void ReflowController::renderSummaryScreen(lv_obj_t* parent) {
//     // Clean existing content
//     if (parent && lv_obj_is_valid(parent)) {
//         lv_obj_clean(parent);
//     }
    
//     // Clear the button list
//     buttonsList.clear();
    
//     // Set up container
//     lv_obj_set_size(parent, LV_HOR_RES, LV_VER_RES);
//     lv_obj_set_style_bg_color(parent, lv_color_hex(0x202020), LV_PART_MAIN);
//     lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
//     lv_obj_set_style_pad_row(parent, 15, 0);
    
//     // Title
//     lv_obj_t* title = lv_label_create(parent);
//     lv_label_set_text(title, "Reflow Complete");
//     lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
//     lv_obj_set_style_text_color(title, lv_color_hex(0x00FF00), 0);
    
//     // Time info
//     lv_obj_t* timeInfo = lv_label_create(parent);
//     char timeText[64];
//     uint32_t totalTimeSeconds = (to_ms_since_boot(get_absolute_time()) / 1000); // Use Pico SDK time functions
//     snprintf(timeText, sizeof(timeText), "Total time: %d seconds", (int)(totalTimeSeconds));
//     lv_label_set_text(timeInfo, timeText);
    
//     // Profile summary
//     lv_obj_t* profileInfo = lv_label_create(parent);
//     lv_label_set_text(profileInfo, ("Profile: " + model.getActiveCurve().name).c_str());
    
//     // Max temperature - get max temp from model
//     lv_obj_t* maxTempInfo = lv_label_create(parent);
//     char maxTempText[64];
//     // Use getCurrentTemp() since there isn't a getMaxTemp() function
//     snprintf(maxTempText, sizeof(maxTempText), "Maximum temperature: %.1f°C", getCurrentTemp());
//     lv_label_set_text(maxTempInfo, maxTempText);
    
//     // Return to menu button
//     lv_obj_t* returnBtn = lv_btn_create(parent);
//     lv_obj_set_size(returnBtn, 200, 50);
//     lv_obj_t* returnLabel = lv_label_create(returnBtn);
//     lv_label_set_text(returnLabel, "Return to Menu");
//     lv_obj_center(returnLabel);
    
//     buttonsList.push_back(returnBtn);
//     selectedButton = 0;
//     updateButtonFocus();
// }

// void ReflowController::updateButtonFocus() {
//     if (buttonsList.empty()) return;
    
//     for (int i = 0; i < static_cast<int>(buttonsList.size()); ++i) {
//         lv_obj_t* btn = buttonsList[i];
//         if (!lv_obj_is_valid(btn)) continue;
        
//         if (i == selectedButton) {
//             lv_obj_set_style_bg_opa(btn, LV_OPA_100, 0);
//             lv_obj_set_style_border_width(btn, 3, 0);
//             lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), 0);
//         } else {
//             lv_obj_set_style_bg_opa(btn, LV_OPA_70, 0);
//             lv_obj_set_style_border_width(btn, 0, 0);
//         }
//     }
// }

// // Implement the willUnload method
// void ReflowController::willUnload() {
//     // Stop any active timer
//     if (updateTimer) {
//         lv_timer_del(updateTimer);
//         updateTimer = nullptr;
//     }
    
//     // Clear button list
//     buttonsList.clear();
    
//     // Reset UI components
//     tempChart = nullptr;
//     tempSeries = nullptr;
//     targetSeries = nullptr;
//     statusLabel = nullptr;
//     tempLabel = nullptr;
//     timeLabel = nullptr;
//     stepLabel = nullptr;
    
//     // Reset button state
//     selectedButton = 0;
// }

// // Update this to use the new pattern
// void ReflowController::updateTimerCallback(lv_timer_t* timer) {
//     // Use getInstance to avoid having to store a pointer in timer->user_data
//     ReflowController::getInstance().updateReflowState();
// }

// // New method to update state when timer calls
// void ReflowController::updateReflowState() {
//     // Update UI based on current state
//     if (state == ReflowState::RUNNING) {
//         updateReflowUI();
//     } else if (state == ReflowState::PRECHECK) {
//         // Update precheck UI
//         if (tempLabel) {
//             char tempText[32];
//             snprintf(tempText, sizeof(tempText), "Current temperature: %.1f°C", currentTemp);
//             lv_label_set_text(tempLabel, tempText);
//         }
//     }
// }

// void ReflowController::updateReflowUI() {
//     // Update UI based on current state
//     switch (state) {
//         case ReflowState::PRECHECK:
//             if (tempLabel) {
//                 char tempText[32];
//                 snprintf(tempText, sizeof(tempText), "Current temperature: %.1f°C", currentTemp);
//                 lv_label_set_text(tempLabel, tempText);
//             }
            
//             if (statusLabel) {
//                 if (currentTemp <= model.getActiveCurve().minimumStartTempC) {
//                     lv_label_set_text(statusLabel, "Ready to start! Press START to begin.");
//                     lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00FF00), 0);
//                     confirmButtonActive = true;
//                 } else {
//                     char statusText[64];
//                     snprintf(statusText, sizeof(statusText), 
//                              "Oven too hot (%.1f°C). Must be below %.1f°C",
//                              currentTemp, model.getActiveCurve().minimumStartTempC);
//                     lv_label_set_text(statusLabel, statusText);
//                     lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFF0000), 0);
//                     confirmButtonActive = false;
//                 }
//             }
            
//             updateButtonFocus();
//             break;
            
//         case ReflowState::RUNNING:
//             if (tempLabel) {
//                 char tempText[32];
//                 snprintf(tempText, sizeof(tempText), "Temp: %.1f°C", currentTemp);
//                 lv_label_set_text(tempLabel, tempText);
//             }
            
//             if (timeLabel) {
//                 char timeText[32];
//                 float elapsed = getElapsedMsInStep() / 1000.0f;
//                 snprintf(timeText, sizeof(timeText), "Time: %.1fs", elapsed);
//                 lv_label_set_text(timeLabel, timeText);
//             }
            
//             if (stepLabel) {
//                 char stepText[64];
//                 snprintf(stepText, sizeof(stepText), "Step: %s (%d/%d)", 
//                          model.getCurrentStep().label.c_str(),
//                          model.getCurrentStepIndex() + 1,
//                          (int)model.getActiveCurve().steps.size());
//                 lv_label_set_text(stepLabel, stepText);
//             }
            
//             // Update chart
//             if (tempChart && tempSeries && targetSeries) {
//                 lv_chart_set_next_value(tempChart, tempSeries, (int)currentTemp);
//                 lv_chart_set_next_value(tempChart, targetSeries, (int)getCurrentTargetTemp());
//             }
//             break;
            
//         case ReflowState::COMPLETE:
//             // Static display, no updates needed
//             break;
//     }
// }

// void ReflowController::onEncoderPress() {
//     BuzzerService::getInstance().playMediumTone(300);
    
//     if (buttonsList.empty()) return;
    
//     // Get the selected button and trigger its action
//     lv_obj_t* selectedBtn = buttonsList[selectedButton];
    
//     if (state == ReflowState::PRECHECK) {
//         if (selectedButton == 0) { // Start button
//             // Check if start conditions are met
//             if (currentTemp <= model.getActiveCurve().minimumStartTempC) {
//                 confirmStart();
//             }
//         } else if (selectedButton == 1) { // Cancel button
//             returnToMainMenu();
//         }
//     } else if (state == ReflowState::RUNNING) {
//         // Only one button in running view - cancel
//         cancel();
//     } else if (state == ReflowState::COMPLETE) {
//         // Only one button in summary - return to menu
//         returnToMainMenu();
//     }
// }

// void ReflowController::onEncoderUp() {
//     if (buttonsList.size() > 1 && selectedButton > 0) {
//         selectedButton--;
//         updateButtonFocus();
//     }
// }

// void ReflowController::onEncoderDown() {
//     if (buttonsList.size() > 1 && selectedButton < static_cast<int>(buttonsList.size()) - 1) {
//         selectedButton++;
//         updateButtonFocus();
//     }
// }

// void ReflowController::onEncoderLongPress() {
//     // Long press always cancels and returns to menu
//     if (state != ReflowState::COMPLETE) {
//         cancel();
//     } else {
//         returnToMainMenu();
//     }
// }

// void ReflowController::requestStart() {
//     state = ReflowState::PRECHECK;
//     model.resetProgress();
//     invalidateView(); // Mark as dirty to trigger re-render
// }

// void ReflowController::confirmStart() {
//     if (!confirmButtonActive) return;
    
//     BuzzerService::getInstance().playHighTone(2000);
//     state = ReflowState::RUNNING;
//     model.resetProgress();
//     stepStartTimeMs = to_ms_since_boot(get_absolute_time());
    
//     // Start the reflow process task
//     xTaskCreate(reflowTaskFunc, "ReflowTask", 2048, this, 2, &reflowTask);
    
//     // Mark as dirty to trigger re-render with process screen
//     invalidateView();
// }

// void ReflowController::cancel() {
//     BuzzerService::getInstance().playLowTone(2000);
    
//     // Stop temperature control
//     TemperatureControlService::getInstance().stopHeating();
    
//     // Stop the reflow task if it's running
//     if (reflowTask) {
//         vTaskDelete(reflowTask);
//         reflowTask = nullptr;
//     }
    
//     // Return to main menu
//     returnToMainMenu();
// }

// void ReflowController::returnToMainMenu() {
//     navigateTo("home", 300, TransitionDirection::SLIDE_IN_RIGHT);
// }

// void ReflowController::reflowTaskFunc(void* param) {
//     ReflowController* controller = static_cast<ReflowController*>(param);
//     controller->runReflowProcess();
//     vTaskDelete(nullptr);
// }

// void ReflowController::runReflowProcess() {
//     // Reset step timer
//     stepStartTimeMs = to_ms_since_boot(get_absolute_time());
    
//     while (!model.isComplete()) {
//         // Get current step parameters
//         const ReflowStep& step = model.getCurrentStep();
//         float targetTemp = getCurrentTargetTemp();
        
//         // Set heater to reach target
//         TemperatureControlService::getInstance().setTargetTemperature(targetTemp);
        
//         // Check if current step is complete
//         float elapsed = getElapsedMsInStep();
//         if (elapsed >= step.durationMs) {
//             BuzzerService::getInstance().playMediumTone(1000);
//             model.advanceStep();
//             stepStartTimeMs = to_ms_since_boot(get_absolute_time());
//         }
        
//         // Delay before next update
//         vTaskDelay(pdMS_TO_TICKS(250));
//     }
    
//     // Process complete
//     BuzzerService::getInstance().playHighTone(2000);
//     state = ReflowState::COMPLETE;
    
//     // Stop heating
//     TemperatureControlService::getInstance().stopHeating();
    
//     // Mark as dirty to trigger re-render with summary screen
//     invalidateView();
// }

// ReflowModel& ReflowController::getModel() {
//     return model;
// }

// const ReflowState& ReflowController::getState() const {
//     return state;
// }

// void ReflowController::setCurrentTemp(float tempC) {
//     currentTemp = tempC;
// }

// float ReflowController::getCurrentTemp() const {
//     return currentTemp;
// }

// float ReflowController::getElapsedMsInStep() const {
//     return to_ms_since_boot(get_absolute_time()) - stepStartTimeMs;
// }

// float ReflowController::getCurrentTargetTemp() const {
//     const auto& step = model.getCurrentStep();
//     float elapsed = getElapsedMsInStep();
//     float ratio = elapsed / step.durationMs;
//     if (ratio > 1.0f) ratio = 1.0f;

//     float previous = (model.getCurrentStepIndex() == 0)
//                      ? currentTemp
//                      : model.getActiveCurve().steps[model.getCurrentStepIndex() - 1].targetTempC;

//     return previous + (step.targetTempC - previous) * ratio;
// }

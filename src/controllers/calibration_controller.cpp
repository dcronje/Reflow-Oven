// #include "controllers/calibration_controller.h"
// #include "services/calibration_service.h"
// #include "services/temperature_control_service.h"
// #include "services/sensor_service.h"
// #include "services/door_service.h"
// #include "services/buzzer_service.h"
// #include <cmath>
// #include <sstream>
// #include <iomanip>
// #include <functional>

// CalibrationController& CalibrationController::getInstance() {
//     static CalibrationController instance;
//     return instance;
// }

// CalibrationController::CalibrationController() {
//     // Initialize with default values
// }

// void CalibrationController::init() {
//     // Initialize any controller resources
// }

// void CalibrationController::render(lv_obj_t* parent) {
//     // Render UI based on current view state directly to parent
//     switch (currentView) {
//         case CalibrationView::MAIN_MENU:
//             renderMainPage(parent);
//             break;
            
//         case CalibrationView::SENSOR_CALIBRATION:
//             renderSensorCalibrationPage(parent);
//             break;
            
//         case CalibrationView::THERMAL_CALIBRATION:
//             renderThermalCalibrationPage(parent);
//             break;
            
//         case CalibrationView::DOOR_CALIBRATION:
//             renderDoorCalibrationPage(parent);
//             break;
            
//         case CalibrationView::SUMMARY:
//             renderSummaryPage(parent);
//             break;
//     }
    
//     // Set up timer for dynamic updates if not already set
//     if (!updateTimer) {
//         updateTimer = lv_timer_create(updateTimerCallback, 500, nullptr);
//     }
// }

// void CalibrationController::setView(CalibrationView view) {
//     // Only update if the view actually changed
//     if (currentView != view) {
//         currentView = view;
//         // Mark as dirty to trigger a re-render
//         invalidateView();
//     }
// }

// void CalibrationController::renderMainPage(lv_obj_t* parent) {
//     // Clean existing content
//     if (parent && lv_obj_is_valid(parent)) {
//         lv_obj_clean(parent);
//     }
    
//     // Clear the button list since we're recreating them
//     buttonsList.clear();
    
//     // Set up flex container for menu items
//     lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
//     lv_obj_set_style_pad_row(parent, 10, 0);
//     lv_obj_set_style_bg_color(parent, lv_color_hex(0x202020), LV_PART_MAIN);
    
//     // Title label
//     lv_obj_t* title = lv_label_create(parent);
//     lv_label_set_text(title, "Calibration");
//     lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
//     lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
//     lv_obj_set_style_pad_bottom(title, 20, 0);
    
//     // Menu items
//     const char* items[] = {
//         "Sensor Calibration",
//         "Thermal Calibration",
//         "Door Calibration",
//         "View Calibration Summary",
//         "Return to Home"
//     };
    
//     for (int i = 0; i < 5; ++i) {
//         lv_obj_t* btn = lv_btn_create(parent);
//         lv_obj_set_size(btn, 200, 40);
        
//         lv_obj_t* label = lv_label_create(btn);
//         lv_label_set_text(label, items[i]);
//         lv_obj_center(label);
        
//         if (i == selectedMenuIndex) {
//             lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
//             lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
//         } else {
//             lv_obj_set_style_bg_color(btn, lv_color_hex(0x404040), LV_PART_MAIN);
//             lv_obj_set_style_text_color(label, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
//         }
        
//         buttonsList.push_back(btn);
//     }
// }

// void CalibrationController::updateMenuButtonFocus() {
//     if (buttonsList.empty()) return;
    
//     for (int i = 0; i < static_cast<int>(buttonsList.size()); ++i) {
//         lv_obj_t* btn = buttonsList[i];
//         if (!lv_obj_is_valid(btn)) continue;
        
//         if (i == selectedMenuIndex) {
//             lv_obj_set_style_bg_color(btn, lv_color_hex(0x0080FF), LV_PART_MAIN);
//             lv_obj_t* label = lv_obj_get_child(btn, 0);
//             if (label) lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
//         } else {
//             lv_obj_set_style_bg_color(btn, lv_color_hex(0x404040), LV_PART_MAIN);
//             lv_obj_t* label = lv_obj_get_child(btn, 0);
//             if (label) lv_obj_set_style_text_color(label, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
//         }
//     }
// }

// void CalibrationController::startSensorCalibration() {
//     CalibrationService::getInstance().startSensorCalibration();
//     setView(CalibrationView::SENSOR_CALIBRATION);
//     BuzzerService::getInstance().playMediumTone(300);
    
//     // Reset UI elements
//     lv_label_set_text(statusLabel, "Starting sensor calibration...");
//     lv_label_set_text(temperatureLabel, "Temperature: --°C");
//     lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);
//     lv_label_set_text(timeLabel, "Time remaining: -- sec");
    
//     // Reset door calibration step
//     doorStep = DoorCalibrationStep::OPEN_POSITION;
// }

// void CalibrationController::startThermalCalibration() {
//     CalibrationService::getInstance().startThermalCalibration();
//     setView(CalibrationView::THERMAL_CALIBRATION);
//     BuzzerService::getInstance().playMediumTone(300);
    
//     // Reset door calibration step
//     doorStep = DoorCalibrationStep::OPEN_POSITION;
// }

// void CalibrationController::startDoorCalibration() {
//     // Reset the door calibration step
//     doorStep = DoorCalibrationStep::OPEN_POSITION;
    
//     // Enable servo for calibration
//     DoorService::getInstance().enableServo();
    
//     // Set to a safe middle position if not already in a known safe state
//     if (!DoorService::getInstance().isFullyOpen() && !DoorService::getInstance().isFullyClosed()) {
//         DoorService::getInstance().setRawAngle(90); // Middle position
//     }
    
//     // Start the calibration service
//     CalibrationService::getInstance().startDoorCalibration();
    
//     // Navigate to the door calibration page
//     setView(CalibrationView::DOOR_CALIBRATION);
    
//     // Update the step label
//     lv_label_set_text(doorStepLabel, "Step: Set Open Position");
    
//     // Update the position label
//     char posText[32];
//     snprintf(posText, sizeof(posText), "Position: %.1f°", getDoorCurrentPosition());
//     lv_label_set_text(doorPositionLabel, posText);
    
//     BuzzerService::getInstance().playMediumTone(300);
// }

// void CalibrationController::stopCalibration() {
//     // Disable servo when calibration stops
//     DoorService::getInstance().disableServo();
//     CalibrationService::getInstance().stopCalibration();
    
//     // Return to calibration menu
//     returnToCalibrationMenu();
// }

// bool CalibrationController::isCalibrated() const {
//     return model.isCalibrated();
// }

// const CalibrationData& CalibrationController::getCalibrationData() const {
//     return model.getCalibrationData();
// }

// const CalibrationState& CalibrationController::getCurrentState() const {
//     return model.getState();
// }

// ThermalCalibrationSummary CalibrationController::getThermalSummary() const {
//     return model.getCalibrationData().thermalSummary;
// }

// float CalibrationController::getExpectedHeatingRate(float powerPercent) const {
//     return CalibrationService::getInstance().getExpectedHeatingRate(powerPercent);
// }

// float CalibrationController::getExpectedCoolingRate(float fanPercent) const {
//     return CalibrationService::getInstance().getExpectedCoolingRate(fanPercent);
// }

// void CalibrationController::viewCalibrationSummary() {
//     BuzzerService::getInstance().playMediumTone(300);
//     setView(CalibrationView::SUMMARY);
// }

// void CalibrationController::returnToCalibrationMenu() {
//     BuzzerService::getInstance().playMediumTone(300);
//     setView(CalibrationView::MAIN_MENU);
// }

// bool CalibrationController::checkTemperatureDifference() const {
//     auto& tempService = TemperatureControlService::getInstance();
//     auto& sensorService = SensorService::getInstance();
    
//     float currentTemp = tempService.getTemperature();
//     float ambientTemp = sensorService.getState().ambientTemp;
    
//     return (std::abs(currentTemp - ambientTemp) <= 5.0f); 
// }

// TemperatureData CalibrationController::getCurrentTemperatures() const {
//     auto& tempService = TemperatureControlService::getInstance();
//     auto& sensorService = SensorService::getInstance();
    
//     return {
//         .current = tempService.getTemperature(),
//         .ambient = sensorService.getState().ambientTemp
//     };
// }

// void CalibrationController::setDoorOpenPosition(float position) {
//     CalibrationService::getInstance().setDoorOpenPosition(position);
// }

// void CalibrationController::setDoorClosedPosition(float position) {
//     CalibrationService::getInstance().setDoorClosedPosition(position);
// }

// bool CalibrationController::isDoorCalibrated() const {
//     return CalibrationService::getInstance().isDoorCalibrated();
// }

// float CalibrationController::getDoorOpenPosition() const {
//     return CalibrationService::getInstance().getDoorOpenPosition();
// }

// float CalibrationController::getDoorClosedPosition() const {
//     return CalibrationService::getInstance().getDoorClosedPosition();
// }

// float CalibrationController::getDoorCurrentPosition() const {
//     return static_cast<float>(DoorService::getInstance().getCurrentAngle());
// }

// void CalibrationController::moveDoorUp() {
//     uint8_t current = DoorService::getInstance().getCurrentAngle();
//     DoorService::getInstance().setRawAngle(current + 1);
// }

// void CalibrationController::moveDoorDown() {
//     uint8_t current = DoorService::getInstance().getCurrentAngle();
//     DoorService::getInstance().setRawAngle(current - 1);
// }

// void CalibrationController::renderSensorCalibrationPage(lv_obj_t* parent) {
//     // Clean existing content
//     if (parent && lv_obj_is_valid(parent)) {
//         lv_obj_clean(parent);
//     }
    
//     // Set background color
//     lv_obj_set_style_bg_color(parent, lv_color_hex(0x202020), LV_PART_MAIN);
    
//     // Set up flex container
//     lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
//     lv_obj_set_style_pad_all(parent, 10, 0);
    
//     // Title label
//     lv_obj_t* title = lv_label_create(parent);
//     lv_label_set_text(title, "Sensor Calibration");
//     lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
//     lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
//     lv_obj_set_style_pad_bottom(title, 20, 0);
    
//     // Status section
//     lv_obj_t* statusSection = lv_obj_create(parent);
//     lv_obj_set_size(statusSection, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_style_bg_color(statusSection, lv_color_hex(0x303030), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(statusSection, 10, 0);
//     lv_obj_set_style_border_width(statusSection, 0, 0);
//     lv_obj_set_flex_flow(statusSection, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_style_pad_row(statusSection, 10, 0);
    
//     // Add status labels
//     statusLabel = lv_label_create(statusSection);
//     lv_label_set_text(statusLabel, "Waiting to start...");
//     lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
//     temperatureLabel = lv_label_create(statusSection);
//     lv_label_set_text(temperatureLabel, "Temperature: --°C");
//     lv_obj_set_style_text_color(temperatureLabel, lv_color_hex(0xFFCC00), LV_PART_MAIN);
    
//     // Progress section
//     lv_obj_t* progressSection = lv_obj_create(parent);
//     lv_obj_set_size(progressSection, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_style_bg_color(progressSection, lv_color_hex(0x303030), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(progressSection, 10, 0);
//     lv_obj_set_style_border_width(progressSection, 0, 0);
//     lv_obj_set_flex_flow(progressSection, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_style_pad_row(progressSection, 10, 0);
    
//     // Create progress bar
//     progressBar = lv_bar_create(progressSection);
//     lv_obj_set_size(progressBar, LV_PCT(100), 15);
//     lv_bar_set_range(progressBar, 0, 100);
//     lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);
    
//     // Create time remaining label
//     timeLabel = lv_label_create(progressSection);
//     lv_label_set_text(timeLabel, "Time remaining: -- sec");
//     lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    
//     // Control section
//     lv_obj_t* controlSection = lv_obj_create(parent);
//     lv_obj_set_size(controlSection, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_style_bg_color(controlSection, lv_color_hex(0x303030), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(controlSection, 10, 0);
//     lv_obj_set_style_border_width(controlSection, 0, 0);
    
//     // Cancel button
//     lv_obj_t* cancelBtn = lv_btn_create(controlSection);
//     lv_obj_set_size(cancelBtn, LV_PCT(100), 40);
//     lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xDD3333), LV_PART_MAIN);
    
//     lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
//     lv_label_set_text(cancelLabel, "Cancel Calibration");
//     lv_obj_center(cancelLabel);
    
//     lv_obj_add_event_cb(cancelBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         controller->stopCalibration();
//         controller->returnToCalibrationMenu();
//     }, LV_EVENT_CLICKED, this);
// }

// void CalibrationController::renderThermalCalibrationPage(lv_obj_t* parent) {
//     // Clean existing content
//     if (parent && lv_obj_is_valid(parent)) {
//         lv_obj_clean(parent);
//     }
    
//     // Set background color
//     lv_obj_set_style_bg_color(parent, lv_color_hex(0x202020), LV_PART_MAIN);
    
//     // Set up flex container
//     lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
//     lv_obj_set_style_pad_all(parent, 10, 0);
    
//     // Title label
//     lv_obj_t* title = lv_label_create(parent);
//     lv_label_set_text(title, "Thermal Calibration");
//     lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
//     lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
//     lv_obj_set_style_pad_bottom(title, 20, 0);
    
//     // Status section
//     lv_obj_t* statusSection = lv_obj_create(parent);
//     lv_obj_set_size(statusSection, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_style_bg_color(statusSection, lv_color_hex(0x303030), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(statusSection, 10, 0);
//     lv_obj_set_style_border_width(statusSection, 0, 0);
//     lv_obj_set_flex_flow(statusSection, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_style_pad_row(statusSection, 10, 0);
    
//     // Add status labels (reuse the ones from sensor calibration)
//     lv_obj_t* phaseLabel = lv_label_create(statusSection);
//     lv_label_set_text(phaseLabel, "Phase: --");
//     lv_obj_set_style_text_color(phaseLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
//     lv_obj_t* tempLabel = lv_label_create(statusSection);
//     lv_label_set_text(tempLabel, "Temperature: --°C");
//     lv_obj_set_style_text_color(tempLabel, lv_color_hex(0xFFCC00), LV_PART_MAIN);
    
//     lv_obj_t* powerLabel = lv_label_create(statusSection);
//     lv_label_set_text(powerLabel, "Power: --%");
//     lv_obj_set_style_text_color(powerLabel, lv_color_hex(0xFF6666), LV_PART_MAIN);
    
//     // Progress section
//     lv_obj_t* progressSection = lv_obj_create(parent);
//     lv_obj_set_size(progressSection, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_style_bg_color(progressSection, lv_color_hex(0x303030), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(progressSection, 10, 0);
//     lv_obj_set_style_border_width(progressSection, 0, 0);
//     lv_obj_set_flex_flow(progressSection, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_style_pad_row(progressSection, 10, 0);
    
//     // Create progress bar (reuse the one from sensor calibration)
//     lv_obj_t* bar = lv_bar_create(progressSection);
//     lv_obj_set_size(bar, LV_PCT(100), 15);
//     lv_bar_set_range(bar, 0, 100);
//     lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    
//     // Create time remaining label
//     lv_obj_t* time = lv_label_create(progressSection);
//     lv_label_set_text(time, "Time remaining: -- min -- sec");
//     lv_obj_set_style_text_color(time, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    
//     // Control section
//     lv_obj_t* controlSection = lv_obj_create(parent);
//     lv_obj_set_size(controlSection, LV_PCT(100), LV_SIZE_CONTENT);
//     lv_obj_set_style_bg_color(controlSection, lv_color_hex(0x303030), LV_PART_MAIN);
//     lv_obj_set_style_pad_all(controlSection, 10, 0);
//     lv_obj_set_style_border_width(controlSection, 0, 0);
    
//     // Cancel button
//     lv_obj_t* cancelBtn = lv_btn_create(controlSection);
//     lv_obj_set_size(cancelBtn, LV_PCT(100), 40);
//     lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xDD3333), LV_PART_MAIN);
    
//     lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
//     lv_label_set_text(cancelLabel, "Cancel Calibration");
//     lv_obj_center(cancelLabel);
    
//     lv_obj_add_event_cb(cancelBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         controller->stopCalibration();
//         controller->returnToCalibrationMenu();
//     }, LV_EVENT_CLICKED, this);
// }

// void CalibrationController::renderDoorCalibrationPage(lv_obj_t* parent) {
//     doorCalibrationPage = lv_menu_page_create(parent, "Door Calibration");
    
//     // Create a section for status
//     lv_obj_t* statusSection = addMenuSection(doorCalibrationPage, "Status");
    
//     // Add step indicator
//     doorStepLabel = lv_label_create(statusSection);
//     lv_label_set_text(doorStepLabel, "Step: Set Open Position");
//     lv_obj_set_style_text_color(doorStepLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
//     // Add position indicator
//     doorPositionLabel = lv_label_create(statusSection);
//     lv_label_set_text(doorPositionLabel, "Position: --°");
//     lv_obj_set_style_text_color(doorPositionLabel, lv_color_hex(0xFFCC00), LV_PART_MAIN);
    
//     // Create controls section
//     lv_obj_t* controlsSection = addMenuSection(doorCalibrationPage, "Position Controls");
    
//     // Create container for up/down buttons
//     lv_obj_t* btnContainer = lv_obj_create(controlsSection);
//     lv_obj_remove_style_all(btnContainer);
//     lv_obj_set_size(btnContainer, LV_PCT(100), 40);
//     lv_obj_set_flex_flow(btnContainer, LV_FLEX_FLOW_ROW);
//     lv_obj_set_flex_align(btnContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
//     // Create up button
//     lv_obj_t* upBtn = lv_btn_create(btnContainer);
//     lv_obj_set_size(upBtn, 80, 40);
//     lv_obj_t* upLabel = lv_label_create(upBtn);
//     lv_label_set_text(upLabel, "Up");
//     lv_obj_center(upLabel);
//     lv_obj_add_event_cb(upBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         controller->moveDoorUp();
        
//         // Update position label
//         char posText[32];
//         snprintf(posText, sizeof(posText), "Position: %.1f°", controller->getDoorCurrentPosition());
//         lv_label_set_text(controller->doorPositionLabel, posText);
//     }, LV_EVENT_CLICKED, this);
    
//     // Create down button
//     lv_obj_t* downBtn = lv_btn_create(btnContainer);
//     lv_obj_set_size(downBtn, 80, 40);
//     lv_obj_t* downLabel = lv_label_create(downBtn);
//     lv_label_set_text(downLabel, "Down");
//     lv_obj_center(downLabel);
//     lv_obj_add_event_cb(downBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         controller->moveDoorDown();
        
//         // Update position label
//         char posText[32];
//         snprintf(posText, sizeof(posText), "Position: %.1f°", controller->getDoorCurrentPosition());
//         lv_label_set_text(controller->doorPositionLabel, posText);
//     }, LV_EVENT_CLICKED, this);
    
//     // Create actions section
//     lv_obj_t* actionsSection = addMenuSection(doorCalibrationPage, "Actions");
    
//     // Set position button
//     lv_obj_t* setBtn = lv_btn_create(actionsSection);
//     lv_obj_set_width(setBtn, LV_PCT(100));
//     lv_obj_t* setBtnLabel = lv_label_create(setBtn);
//     lv_label_set_text(setBtnLabel, "Set Current Position");
//     lv_obj_center(setBtnLabel);
//     lv_obj_add_event_cb(setBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         float currentPos = controller->getDoorCurrentPosition();
        
//         if (controller->doorStep == DoorCalibrationStep::OPEN_POSITION) {
//             controller->setDoorOpenPosition(currentPos);
//             lv_label_set_text(controller->doorStepLabel, "Step: Set Closed Position");
//             controller->doorStep = DoorCalibrationStep::CLOSED_POSITION;
//         } else if (controller->doorStep == DoorCalibrationStep::CLOSED_POSITION) {
//             controller->setDoorClosedPosition(currentPos);
//             lv_label_set_text(controller->doorStepLabel, "Step: Complete");
//             controller->doorStep = DoorCalibrationStep::COMPLETE;
//             controller->returnToCalibrationMenu();
//         }
//     }, LV_EVENT_CLICKED, this);
    
//     // Cancel button
//     lv_obj_t* cancelBtn = lv_btn_create(actionsSection);
//     lv_obj_set_width(cancelBtn, LV_PCT(100));
//     lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xDD3333), LV_PART_MAIN);
//     lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
//     lv_label_set_text(cancelLabel, "Cancel");
//     lv_obj_center(cancelLabel);
//     lv_obj_add_event_cb(cancelBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         controller->stopCalibration();
//         controller->returnToCalibrationMenu();
//     }, LV_EVENT_CLICKED, this);
// }

// void CalibrationController::renderSummaryPage(lv_obj_t* parent) {
//     summaryPage = lv_menu_page_create(parent, "Calibration Summary");
    
//     // Add sensor calibration section
//     lv_obj_t* sensorSection = addMenuSection(summaryPage, "Sensor Calibration");
    
//     lv_obj_t* sensorStatus = lv_label_create(sensorSection);
//     lv_label_set_text(sensorStatus, isCalibrated() ? "Status: Calibrated" : "Status: Not Calibrated");
//     lv_obj_set_style_text_color(sensorStatus, isCalibrated() ? lv_color_hex(0x55FF55) : lv_color_hex(0xFF5555), LV_PART_MAIN);
    
//     lv_obj_t* sensorOffset = lv_label_create(sensorSection);
//     char offsetText[32];
//     snprintf(offsetText, sizeof(offsetText), "Sensor Offset: %.1f°C", getCalibrationData().sensorOffset);
//     lv_label_set_text(sensorOffset, offsetText);
    
//     // Add thermal calibration section
//     lv_obj_t* thermalSection = addMenuSection(summaryPage, "Thermal Calibration");
    
//     // Add some heating/cooling rate examples
//     auto summary = getThermalSummary();
    
//     lv_obj_t* heatingRateLabel = lv_label_create(thermalSection);
//     char heatingText[64];
//     float rate50 = summary.getHeatingRateAtTempAndPowerPercent(0, 50.0f);
//     snprintf(heatingText, sizeof(heatingText), "Heating Rate (50%%): %.1f°C/s", rate50);
//     lv_label_set_text(heatingRateLabel, heatingText);
    
//     lv_obj_t* coolingRateLabel = lv_label_create(thermalSection);
//     char coolingText[64];
//     float coolRate50 = summary.getCoolingRateAtTempAndPowerPercent(0, 50.0f);
//     snprintf(coolingText, sizeof(coolingText), "Cooling Rate (50%%): %.1f°C/s", coolRate50);
//     lv_label_set_text(coolingRateLabel, coolingText);
    
//     // Add door calibration section
//     lv_obj_t* doorSection = addMenuSection(summaryPage, "Door Calibration");
    
//     lv_obj_t* doorStatus = lv_label_create(doorSection);
//     lv_label_set_text(doorStatus, isDoorCalibrated() ? "Status: Calibrated" : "Status: Not Calibrated");
//     lv_obj_set_style_text_color(doorStatus, isDoorCalibrated() ? lv_color_hex(0x55FF55) : lv_color_hex(0xFF5555), LV_PART_MAIN);
    
//     if (isDoorCalibrated()) {
//         lv_obj_t* openPos = lv_label_create(doorSection);
//         char openText[32];
//         snprintf(openText, sizeof(openText), "Open Position: %.1f°", getDoorOpenPosition());
//         lv_label_set_text(openPos, openText);
        
//         lv_obj_t* closedPos = lv_label_create(doorSection);
//         char closedText[32];
//         snprintf(closedText, sizeof(closedText), "Closed Position: %.1f°", getDoorClosedPosition());
//         lv_label_set_text(closedPos, closedText);
//     }
    
//     // Add last calibration time
//     lv_obj_t* timeSection = addMenuSection(summaryPage, "Last Calibration");
    
//     lv_obj_t* timeLabel = lv_label_create(timeSection);
//     char timeText[64];
//     uint32_t lastTime = getCalibrationData().lastCalibrationTime;
//     if (lastTime > 0) {
//         uint32_t hours = lastTime / 3600000;
//         uint32_t minutes = (lastTime % 3600000) / 60000;
//         snprintf(timeText, sizeof(timeText), "%d hours, %d minutes ago", hours, minutes);
//     } else {
//         snprintf(timeText, sizeof(timeText), "Never");
//     }
//     lv_label_set_text(timeLabel, timeText);
    
//     // Create back button section
//     lv_obj_t* backSection = addMenuSection(summaryPage, "");
    
//     lv_obj_t* backBtn = lv_btn_create(backSection);
//     lv_obj_set_width(backBtn, LV_PCT(100));
//     lv_obj_t* backLabel = lv_label_create(backBtn);
//     lv_label_set_text(backLabel, "Back to Calibration Menu");
//     lv_obj_center(backLabel);
//     lv_obj_add_event_cb(backBtn, [](lv_event_t* e) {
//         auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
//         controller->returnToCalibrationMenu();
//     }, LV_EVENT_CLICKED, this);
// }

// // Fix the timer callback to properly use the timer without access to its internal structure
// void CalibrationController::updateTimerCallback(lv_timer_t* timer) {
//     // In this callback pattern, we just use the instance itself without directly accessing 
//     // the timer's user_data structure to avoid the incomplete type issue
//     CalibrationController::getInstance().updateCalibrationState();
// }

// // This method gets called by the timer callback and performs the actual update
// void CalibrationController::updateCalibrationState() {
//     // Check which view is active and update accordingly
//     switch (currentView) {
//         case CalibrationView::SENSOR_CALIBRATION:
//         case CalibrationView::THERMAL_CALIBRATION:
//             updateCalibrationProgress();
//             break;
            
//         case CalibrationView::DOOR_CALIBRATION:
//             updateDoorCalibrationUI();
//             break;
            
//         default:
//             // No updates needed for other views
//             break;
//     }
// }

// void CalibrationController::updateCalibrationProgress() {
//     // Update model from service
//     model.updateFromService();
    
//     const auto& state = model.getState();
    
//     // Update UI based on the current calibration phase
//     if (state.phase == CalibrationPhase::TEMPERATURE_CALIBRATION ||
//         state.phase == CalibrationPhase::HEATING_CALIBRATION ||
//         state.phase == CalibrationPhase::COOLING_CALIBRATION) {
        
//         // Update status label
//         const char* phaseText = "";
//         switch (state.phase) {
//             case CalibrationPhase::TEMPERATURE_CALIBRATION:
//                 phaseText = "Temperature Calibration";
//                 break;
//             case CalibrationPhase::HEATING_CALIBRATION:
//                 phaseText = "Heating Calibration";
//                 break;
//             case CalibrationPhase::COOLING_CALIBRATION:
//                 phaseText = "Cooling Calibration";
//                 break;
//             default:
//                 phaseText = "Unknown";
//                 break;
//         }
        
//         if (statusLabel) {
//             lv_label_set_text(statusLabel, phaseText);
//         }
        
//         // Update temperature label
//         if (temperatureLabel) {
//             char tempText[32];
//             snprintf(tempText, sizeof(tempText), "Temperature: %.1f°C", state.currentTemp);
//             lv_label_set_text(temperatureLabel, tempText);
//         }
        
//         // Update progress bar
//         if (progressBar) {
//             int progress = static_cast<int>(state.progress * 100);
//             lv_bar_set_value(progressBar, progress, LV_ANIM_ON);
//         }
        
//         // Update time remaining label
//         if (timeLabel) {
//             char timeText[32];
//             int seconds = state.timeRemainingMs / 1000;
//             snprintf(timeText, sizeof(timeText), "Time remaining: %d sec", seconds);
//             lv_label_set_text(timeLabel, timeText);
//         }
//     }
    
//     // Check for errors
//     if (state.hasError && state.errorMessage) {
//         if (statusLabel) {
//             char errorText[64];
//             snprintf(errorText, sizeof(errorText), "ERROR: %s", state.errorMessage);
//             lv_label_set_text(statusLabel, errorText);
//             lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFF0000), LV_PART_MAIN);
//         }
//     }
// }

// // Update methods for navigation
// void CalibrationController::returnToHome() {
//     BuzzerService::getInstance().playMediumTone(300);
//     navigateTo("home", 300, TransitionDirection::SLIDE_IN_RIGHT);
// }

// // Input handlers
// void CalibrationController::onEncoderPress() {
//     BuzzerService::getInstance().playMediumTone(300);
    
//     if (currentView == CalibrationView::MAIN_MENU) {
//         switch (selectedMenuIndex) {
//             case 0: startSensorCalibration(); break;
//             case 1: startThermalCalibration(); break;
//             case 2: startDoorCalibration(); break;
//             case 3: viewCalibrationSummary(); break;
//             case 4: returnToHome(); break;
//         }
//     } else if (currentView == CalibrationView::DOOR_CALIBRATION) {
//         // In door calibration mode, encoder press confirms the current position
//         if (doorStep == DoorCalibrationStep::OPEN_POSITION) {
//             setDoorOpenPosition(DoorService::getInstance().getCurrentAngle());
//             doorStep = DoorCalibrationStep::CLOSED_POSITION;
//             updateDoorCalibrationUI();
//         } else if (doorStep == DoorCalibrationStep::CLOSED_POSITION) {
//             setDoorClosedPosition(DoorService::getInstance().getCurrentAngle());
//             doorStep = DoorCalibrationStep::COMPLETE;
//             updateDoorCalibrationUI();
//         } else if (doorStep == DoorCalibrationStep::COMPLETE) {
//             returnToCalibrationMenu();
//         }
//     }
// }

// void CalibrationController::onEncoderLongPress() {
//     if (currentView != CalibrationView::MAIN_MENU) {
//         // Go back to the main calibration page
//         returnToCalibrationMenu();
//     } else {
//         // Go back to home
//         returnToHome();
//     }
// }

// void CalibrationController::onEncoderUp() {
//     if (currentView == CalibrationView::MAIN_MENU) {
//         if (selectedMenuIndex > 0) {
//             selectedMenuIndex--;
//             updateMenuButtonFocus();
//         }
//     } else if (currentView == CalibrationView::DOOR_CALIBRATION) {
//         // During door calibration, encoder turns adjust the door position
//         moveDoorUp();
//     }
// }

// void CalibrationController::onEncoderDown() {
//     if (currentView == CalibrationView::MAIN_MENU) {
//         if (selectedMenuIndex < static_cast<int>(buttonsList.size()) - 1) {
//             selectedMenuIndex++;
//             updateMenuButtonFocus();
//         }
//     } else if (currentView == CalibrationView::DOOR_CALIBRATION) {
//         // During door calibration, encoder turns adjust the door position
//         moveDoorDown();
//     }
// }

// void CalibrationController::willUnload() {
//     // Stop any active timer
//     if (updateTimer) {
//         lv_timer_del(updateTimer);
//         updateTimer = nullptr;
//     }
    
//     // Clear button list
//     buttonsList.clear();
    
//     // Reset UI components
//     progressBar = nullptr;
//     progressLabel = nullptr;
//     temperatureLabel = nullptr;
//     statusLabel = nullptr;
//     timeLabel = nullptr;
//     doorPositionLabel = nullptr;
//     doorStepLabel = nullptr;
    
//     // Reset the current view to main menu for the next time this controller is loaded
//     currentView = CalibrationView::MAIN_MENU;
//     selectedMenuIndex = 0;
// }

// // Implement the missing updateDoorCalibrationUI method
// void CalibrationController::updateDoorCalibrationUI() {
//     // Update position label with current door position
//     if (doorPositionLabel) {
//         char posText[32];
//         snprintf(posText, sizeof(posText), "Position: %.1f°", getDoorCurrentPosition());
//         lv_label_set_text(doorPositionLabel, posText);
//     }
    
//     // Update step label based on current calibration step
//     if (doorStepLabel) {
//         switch (doorStep) {
//             case DoorCalibrationStep::OPEN_POSITION:
//                 lv_label_set_text(doorStepLabel, "Step: Set Open Position");
//                 break;
                
//             case DoorCalibrationStep::CLOSED_POSITION:
//                 lv_label_set_text(doorStepLabel, "Step: Set Closed Position");
//                 break;
                
//             case DoorCalibrationStep::COMPLETE:
//                 lv_label_set_text(doorStepLabel, "Step: Complete");
//                 break;
//         }
//     }
    
//     // Invalidate view if there are major UI changes
//     // (we don't need to do this for simple label updates)
// }

// // Implement addMenuSection helper method
// lv_obj_t* CalibrationController::addMenuSection(lv_obj_t* page, const char* title) {
//     lv_obj_t* section = lv_menu_section_create(page);
    
//     if (title != nullptr) {
//         lv_obj_t* sectionHeader = lv_label_create(section);
//         lv_label_set_text(sectionHeader, title);
//         lv_obj_set_style_text_color(sectionHeader, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
//         lv_obj_set_style_pad_top(sectionHeader, 10, LV_PART_MAIN);
//         lv_obj_set_style_pad_bottom(sectionHeader, 5, LV_PART_MAIN);
//     }
    
//     return section;
// }

// // Implement addMenuItem helper method
// lv_obj_t* CalibrationController::addMenuItem(lv_obj_t* section, const char* text, lv_event_cb_t callback) {
//     lv_obj_t* item = lv_menu_cont_create(section);
    
//     lv_obj_t* label = lv_label_create(item);
//     lv_label_set_text(label, text);
//     lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
//     if (callback) {
//         lv_obj_add_event_cb(item, callback, LV_EVENT_CLICKED, this);
//     }
    
//     return item;
// }


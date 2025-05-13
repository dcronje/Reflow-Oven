#include "controllers/calibration_controller.h"
#include "services/calibration_service.h"
#include "services/temperature_control_service.h"
#include "services/sensor_service.h"
#include "services/door_service.h"
#include "services/buzzer_service.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <functional>

CalibrationController& CalibrationController::getInstance() {
    static CalibrationController instance;
    return instance;
}

CalibrationController::CalibrationController() {
    // Initialize with default values
}

void CalibrationController::init() {
    // Initialize any controller resources
}

void CalibrationController::render() {
    if (screen == nullptr) {
        // Create the screen
        screen = lv_obj_create(nullptr);
        lv_obj_set_size(screen, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), LV_PART_MAIN);
        lv_obj_set_style_pad_all(screen, 0, 0);
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        
        // Initialize the menu - this is the container for all pages
        renderMenu();
        
        // Create a timer for updating UI elements - pass 'this' directly as the handle
        updateTimer = lv_timer_create(updateTimerCallback, 500, (void*)this);
    }
    
    // Render the current view
    switch (currentView) {
        case CalibrationView::MAIN_MENU:
            if (mainPage == nullptr) {
                renderMainPage();
            }
            lv_menu_set_page(menu, mainPage);
            currentPage = mainPage;
            break;
            
        case CalibrationView::SENSOR_CALIBRATION:
            if (sensorCalibrationPage == nullptr) {
                renderSensorCalibrationPage();
            }
            lv_menu_set_page(menu, sensorCalibrationPage);
            currentPage = sensorCalibrationPage;
            break;
            
        case CalibrationView::THERMAL_CALIBRATION:
            if (thermalCalibrationPage == nullptr) {
                renderThermalCalibrationPage();
            }
            lv_menu_set_page(menu, thermalCalibrationPage);
            currentPage = thermalCalibrationPage;
            break;
            
        case CalibrationView::DOOR_CALIBRATION:
            if (doorCalibrationPage == nullptr) {
                renderDoorCalibrationPage();
            }
            lv_menu_set_page(menu, doorCalibrationPage);
            currentPage = doorCalibrationPage;
            break;
            
        case CalibrationView::SUMMARY:
            if (summaryPage == nullptr) {
                renderSummaryPage();
            }
            lv_menu_set_page(menu, summaryPage);
            currentPage = summaryPage;
            break;
    }
    
    // Load the screen
    lv_scr_load(screen);
}

void CalibrationController::setView(CalibrationView view) {
    currentView = view;
    
    // If the screen is already created, switch to the specified view
    if (screen != nullptr) {
        switch (view) {
            case CalibrationView::MAIN_MENU:
                if (mainPage == nullptr) {
                    renderMainPage();
                }
                lv_menu_set_page(menu, mainPage);
                currentPage = mainPage;
                break;
                
            case CalibrationView::SENSOR_CALIBRATION:
                if (sensorCalibrationPage == nullptr) {
                    renderSensorCalibrationPage();
                }
                lv_menu_set_page(menu, sensorCalibrationPage);
                currentPage = sensorCalibrationPage;
                break;
                
            case CalibrationView::THERMAL_CALIBRATION:
                if (thermalCalibrationPage == nullptr) {
                    renderThermalCalibrationPage();
                }
                lv_menu_set_page(menu, thermalCalibrationPage);
                currentPage = thermalCalibrationPage;
                break;
                
            case CalibrationView::DOOR_CALIBRATION:
                if (doorCalibrationPage == nullptr) {
                    renderDoorCalibrationPage();
                }
                lv_menu_set_page(menu, doorCalibrationPage);
                currentPage = doorCalibrationPage;
                break;
                
            case CalibrationView::SUMMARY:
                if (summaryPage == nullptr) {
                    renderSummaryPage();
                }
                lv_menu_set_page(menu, summaryPage);
                currentPage = summaryPage;
                break;
        }
    }
}

void CalibrationController::renderMenu() {
    menu = lv_menu_create(screen);
    lv_obj_set_size(menu, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(menu, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(menu, lv_color_hex(0x222222), LV_PART_MAIN);
    
    // Configure menu
    lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    
    // Add event handlers for menu
    lv_obj_add_event_cb(menu, [](lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        CalibrationController* controller = (CalibrationController*)lv_event_get_user_data(e);
        
        if (lv_menu_back_button_is_root(controller->menu, obj)) {
            controller->returnToHome();
        }
    }, LV_EVENT_CLICKED, this);
}

void CalibrationController::renderMainPage() {
    mainPage = lv_menu_page_create(menu, "Calibration");
    
    // Add sections
    lv_obj_t* calibrationSection = addMenuSection(mainPage, "Calibration Options");
    
    // Add items to the section
    lv_obj_t* sensorItem = addMenuItem(calibrationSection, "Sensor Calibration");
    lv_obj_add_event_cb(sensorItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->startSensorCalibration();
    }, LV_EVENT_CLICKED, this);
    
    lv_obj_t* thermalItem = addMenuItem(calibrationSection, "Thermal Calibration");
    lv_obj_add_event_cb(thermalItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->startThermalCalibration();
    }, LV_EVENT_CLICKED, this);
    
    lv_obj_t* doorItem = addMenuItem(calibrationSection, "Door Calibration");
    lv_obj_add_event_cb(doorItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->startDoorCalibration();
    }, LV_EVENT_CLICKED, this);
    
    // Add summary and back options
    lv_obj_t* summarySection = addMenuSection(mainPage, "Information");
    
    lv_obj_t* summaryItem = addMenuItem(summarySection, "View Calibration Summary");
    lv_obj_add_event_cb(summaryItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->viewCalibrationSummary();
    }, LV_EVENT_CLICKED, this);
    
    // Add home button
    lv_obj_t* systemSection = addMenuSection(mainPage, "System");
    
    lv_obj_t* homeItem = addMenuItem(systemSection, "Return to Home");
    lv_obj_add_event_cb(homeItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->returnToHome();
    }, LV_EVENT_CLICKED, this);
}

lv_obj_t* CalibrationController::addMenuSection(lv_obj_t* page, const char* title) {
    lv_obj_t* section = lv_menu_section_create(page);
    
    if (title != nullptr) {
        lv_obj_t* sectionHeader = lv_label_create(section);
        lv_label_set_text(sectionHeader, title);
        lv_obj_set_style_text_color(sectionHeader, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
        lv_obj_set_style_pad_top(sectionHeader, 10, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(sectionHeader, 5, LV_PART_MAIN);
    }
    
    return section;
}

lv_obj_t* CalibrationController::addMenuItem(lv_obj_t* section, const char* text, lv_event_cb_t callback) {
    lv_obj_t* item = lv_menu_cont_create(section);
    
    lv_obj_t* label = lv_label_create(item);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    if (callback) {
        lv_obj_add_event_cb(item, callback, LV_EVENT_CLICKED, this);
    }
    
    return item;
}

void CalibrationController::startSensorCalibration() {
    CalibrationService::getInstance().startSensorCalibration();
    setView(CalibrationView::SENSOR_CALIBRATION);
    BuzzerService::getInstance().playMediumTone(300);
    
    // Reset UI elements
    lv_label_set_text(statusLabel, "Starting sensor calibration...");
    lv_label_set_text(temperatureLabel, "Temperature: --°C");
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);
    lv_label_set_text(timeLabel, "Time remaining: -- sec");
    
    // Reset door calibration step
    doorStep = DoorCalibrationStep::OPEN_POSITION;
}

void CalibrationController::startThermalCalibration() {
    CalibrationService::getInstance().startThermalCalibration();
    setView(CalibrationView::THERMAL_CALIBRATION);
    BuzzerService::getInstance().playMediumTone(300);
    
    // Reset door calibration step
    doorStep = DoorCalibrationStep::OPEN_POSITION;
}

void CalibrationController::startDoorCalibration() {
    // Reset the door calibration step
    doorStep = DoorCalibrationStep::OPEN_POSITION;
    
    // Enable servo for calibration
    DoorService::getInstance().enableServo();
    
    // Set to a safe middle position if not already in a known safe state
    if (!DoorService::getInstance().isFullyOpen() && !DoorService::getInstance().isFullyClosed()) {
        DoorService::getInstance().setRawAngle(90); // Middle position
    }
    
    // Start the calibration service
    CalibrationService::getInstance().startDoorCalibration();
    
    // Navigate to the door calibration page
    setView(CalibrationView::DOOR_CALIBRATION);
    
    // Update the step label
    lv_label_set_text(doorStepLabel, "Step: Set Open Position");
    
    // Update the position label
    char posText[32];
    snprintf(posText, sizeof(posText), "Position: %.1f°", getDoorCurrentPosition());
    lv_label_set_text(doorPositionLabel, posText);
    
    BuzzerService::getInstance().playMediumTone(300);
}

void CalibrationController::stopCalibration() {
    // Disable servo when calibration stops
    DoorService::getInstance().disableServo();
    CalibrationService::getInstance().stopCalibration();
    
    // Return to calibration menu
    returnToCalibrationMenu();
}

bool CalibrationController::isCalibrated() const {
    return model.isCalibrated();
}

const CalibrationData& CalibrationController::getCalibrationData() const {
    return model.getCalibrationData();
}

const CalibrationState& CalibrationController::getCurrentState() const {
    return model.getState();
}

ThermalCalibrationSummary CalibrationController::getThermalSummary() const {
    return model.getCalibrationData().thermalSummary;
}

float CalibrationController::getExpectedHeatingRate(float powerPercent) const {
    return CalibrationService::getInstance().getExpectedHeatingRate(powerPercent);
}

float CalibrationController::getExpectedCoolingRate(float fanPercent) const {
    return CalibrationService::getInstance().getExpectedCoolingRate(fanPercent);
}

void CalibrationController::viewCalibrationSummary() {
    BuzzerService::getInstance().playMediumTone(300);
    setView(CalibrationView::SUMMARY);
}

void CalibrationController::returnToCalibrationMenu() {
    BuzzerService::getInstance().playMediumTone(300);
    setView(CalibrationView::MAIN_MENU);
}

bool CalibrationController::checkTemperatureDifference() const {
    auto& tempService = TemperatureControlService::getInstance();
    auto& sensorService = SensorService::getInstance();
    
    float currentTemp = tempService.getTemperature();
    float ambientTemp = sensorService.getState().ambientTemp;
    
    return (std::abs(currentTemp - ambientTemp) <= 5.0f); 
}

TemperatureData CalibrationController::getCurrentTemperatures() const {
    auto& tempService = TemperatureControlService::getInstance();
    auto& sensorService = SensorService::getInstance();
    
    return {
        .current = tempService.getTemperature(),
        .ambient = sensorService.getState().ambientTemp
    };
}

void CalibrationController::setDoorOpenPosition(float position) {
    CalibrationService::getInstance().setDoorOpenPosition(position);
}

void CalibrationController::setDoorClosedPosition(float position) {
    CalibrationService::getInstance().setDoorClosedPosition(position);
}

bool CalibrationController::isDoorCalibrated() const {
    return CalibrationService::getInstance().isDoorCalibrated();
}

float CalibrationController::getDoorOpenPosition() const {
    return CalibrationService::getInstance().getDoorOpenPosition();
}

float CalibrationController::getDoorClosedPosition() const {
    return CalibrationService::getInstance().getDoorClosedPosition();
}

float CalibrationController::getDoorCurrentPosition() const {
    return static_cast<float>(DoorService::getInstance().getCurrentAngle());
}

void CalibrationController::moveDoorUp() {
    uint8_t current = DoorService::getInstance().getCurrentAngle();
    DoorService::getInstance().setRawAngle(current + 1);
}

void CalibrationController::moveDoorDown() {
    uint8_t current = DoorService::getInstance().getCurrentAngle();
    DoorService::getInstance().setRawAngle(current - 1);
}

void CalibrationController::renderSensorCalibrationPage() {
    sensorCalibrationPage = lv_menu_page_create(menu, "Sensor Calibration");
    
    // Create a section for status
    lv_obj_t* statusSection = addMenuSection(sensorCalibrationPage, "Status");
    
    // Add status labels
    statusLabel = lv_label_create(statusSection);
    lv_label_set_text(statusLabel, "Waiting to start...");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    temperatureLabel = lv_label_create(statusSection);
    lv_label_set_text(temperatureLabel, "Temperature: --°C");
    lv_obj_set_style_text_color(temperatureLabel, lv_color_hex(0xFFCC00), LV_PART_MAIN);
    
    // Create progress section
    lv_obj_t* progressSection = addMenuSection(sensorCalibrationPage, "Progress");
    
    // Create progress bar
    progressBar = lv_bar_create(progressSection);
    lv_obj_set_size(progressBar, 200, 15);
    lv_bar_set_range(progressBar, 0, 100);
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);
    
    // Create time remaining label
    timeLabel = lv_label_create(progressSection);
    lv_label_set_text(timeLabel, "Time remaining: -- sec");
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    
    // Create section for controls
    lv_obj_t* controlsSection = addMenuSection(sensorCalibrationPage, "Controls");
    
    // Add button to cancel calibration
    lv_obj_t* cancelItem = addMenuItem(controlsSection, "Cancel Calibration");
    lv_obj_add_event_cb(cancelItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->stopCalibration();
        controller->returnToCalibrationMenu();
    }, LV_EVENT_CLICKED, this);
}

void CalibrationController::renderThermalCalibrationPage() {
    thermalCalibrationPage = lv_menu_page_create(menu, "Thermal Calibration");
    
    // Create a section for status
    lv_obj_t* statusSection = addMenuSection(thermalCalibrationPage, "Status");
    
    // Add status labels (reuse the ones from sensor calibration)
    lv_obj_t* phaseLabel = lv_label_create(statusSection);
    lv_label_set_text(phaseLabel, "Phase: --");
    lv_obj_set_style_text_color(phaseLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_t* tempLabel = lv_label_create(statusSection);
    lv_label_set_text(tempLabel, "Temperature: --°C");
    lv_obj_set_style_text_color(tempLabel, lv_color_hex(0xFFCC00), LV_PART_MAIN);
    
    lv_obj_t* powerLabel = lv_label_create(statusSection);
    lv_label_set_text(powerLabel, "Power: --%");
    lv_obj_set_style_text_color(powerLabel, lv_color_hex(0xFF6666), LV_PART_MAIN);
    
    // Create progress section
    lv_obj_t* progressSection = addMenuSection(thermalCalibrationPage, "Progress");
    
    // Create progress bar (reuse the one from sensor calibration)
    lv_obj_t* bar = lv_bar_create(progressSection);
    lv_obj_set_size(bar, 200, 15);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    
    // Create time remaining label
    lv_obj_t* time = lv_label_create(progressSection);
    lv_label_set_text(time, "Time remaining: -- min -- sec");
    lv_obj_set_style_text_color(time, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    
    // Create section for controls
    lv_obj_t* controlsSection = addMenuSection(thermalCalibrationPage, "Controls");
    
    // Add button to cancel calibration
    lv_obj_t* cancelItem = addMenuItem(controlsSection, "Cancel Calibration");
    lv_obj_add_event_cb(cancelItem, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->stopCalibration();
        controller->returnToCalibrationMenu();
    }, LV_EVENT_CLICKED, this);
}

void CalibrationController::renderDoorCalibrationPage() {
    doorCalibrationPage = lv_menu_page_create(menu, "Door Calibration");
    
    // Create a section for status
    lv_obj_t* statusSection = addMenuSection(doorCalibrationPage, "Status");
    
    // Add step indicator
    doorStepLabel = lv_label_create(statusSection);
    lv_label_set_text(doorStepLabel, "Step: Set Open Position");
    lv_obj_set_style_text_color(doorStepLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    // Add position indicator
    doorPositionLabel = lv_label_create(statusSection);
    lv_label_set_text(doorPositionLabel, "Position: --°");
    lv_obj_set_style_text_color(doorPositionLabel, lv_color_hex(0xFFCC00), LV_PART_MAIN);
    
    // Create controls section
    lv_obj_t* controlsSection = addMenuSection(doorCalibrationPage, "Position Controls");
    
    // Create container for up/down buttons
    lv_obj_t* btnContainer = lv_obj_create(controlsSection);
    lv_obj_remove_style_all(btnContainer);
    lv_obj_set_size(btnContainer, LV_PCT(100), 40);
    lv_obj_set_flex_flow(btnContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btnContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Create up button
    lv_obj_t* upBtn = lv_btn_create(btnContainer);
    lv_obj_set_size(upBtn, 80, 40);
    lv_obj_t* upLabel = lv_label_create(upBtn);
    lv_label_set_text(upLabel, "Up");
    lv_obj_center(upLabel);
    lv_obj_add_event_cb(upBtn, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->moveDoorUp();
        
        // Update position label
        char posText[32];
        snprintf(posText, sizeof(posText), "Position: %.1f°", controller->getDoorCurrentPosition());
        lv_label_set_text(controller->doorPositionLabel, posText);
    }, LV_EVENT_CLICKED, this);
    
    // Create down button
    lv_obj_t* downBtn = lv_btn_create(btnContainer);
    lv_obj_set_size(downBtn, 80, 40);
    lv_obj_t* downLabel = lv_label_create(downBtn);
    lv_label_set_text(downLabel, "Down");
    lv_obj_center(downLabel);
    lv_obj_add_event_cb(downBtn, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->moveDoorDown();
        
        // Update position label
        char posText[32];
        snprintf(posText, sizeof(posText), "Position: %.1f°", controller->getDoorCurrentPosition());
        lv_label_set_text(controller->doorPositionLabel, posText);
    }, LV_EVENT_CLICKED, this);
    
    // Create actions section
    lv_obj_t* actionsSection = addMenuSection(doorCalibrationPage, "Actions");
    
    // Set position button
    lv_obj_t* setBtn = lv_btn_create(actionsSection);
    lv_obj_set_width(setBtn, LV_PCT(100));
    lv_obj_t* setBtnLabel = lv_label_create(setBtn);
    lv_label_set_text(setBtnLabel, "Set Current Position");
    lv_obj_center(setBtnLabel);
    lv_obj_add_event_cb(setBtn, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        float currentPos = controller->getDoorCurrentPosition();
        
        if (controller->doorStep == DoorCalibrationStep::OPEN_POSITION) {
            controller->setDoorOpenPosition(currentPos);
            lv_label_set_text(controller->doorStepLabel, "Step: Set Closed Position");
            controller->doorStep = DoorCalibrationStep::CLOSED_POSITION;
        } else if (controller->doorStep == DoorCalibrationStep::CLOSED_POSITION) {
            controller->setDoorClosedPosition(currentPos);
            lv_label_set_text(controller->doorStepLabel, "Step: Complete");
            controller->doorStep = DoorCalibrationStep::COMPLETE;
            controller->returnToCalibrationMenu();
        }
    }, LV_EVENT_CLICKED, this);
    
    // Cancel button
    lv_obj_t* cancelBtn = lv_btn_create(actionsSection);
    lv_obj_set_width(cancelBtn, LV_PCT(100));
    lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0xDD3333), LV_PART_MAIN);
    lv_obj_t* cancelLabel = lv_label_create(cancelBtn);
    lv_label_set_text(cancelLabel, "Cancel");
    lv_obj_center(cancelLabel);
    lv_obj_add_event_cb(cancelBtn, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->stopCalibration();
        controller->returnToCalibrationMenu();
    }, LV_EVENT_CLICKED, this);
}

void CalibrationController::renderSummaryPage() {
    summaryPage = lv_menu_page_create(menu, "Calibration Summary");
    
    // Add sensor calibration section
    lv_obj_t* sensorSection = addMenuSection(summaryPage, "Sensor Calibration");
    
    lv_obj_t* sensorStatus = lv_label_create(sensorSection);
    lv_label_set_text(sensorStatus, isCalibrated() ? "Status: Calibrated" : "Status: Not Calibrated");
    lv_obj_set_style_text_color(sensorStatus, isCalibrated() ? lv_color_hex(0x55FF55) : lv_color_hex(0xFF5555), LV_PART_MAIN);
    
    lv_obj_t* sensorOffset = lv_label_create(sensorSection);
    char offsetText[32];
    snprintf(offsetText, sizeof(offsetText), "Sensor Offset: %.1f°C", getCalibrationData().sensorOffset);
    lv_label_set_text(sensorOffset, offsetText);
    
    // Add thermal calibration section
    lv_obj_t* thermalSection = addMenuSection(summaryPage, "Thermal Calibration");
    
    // Add some heating/cooling rate examples
    auto summary = getThermalSummary();
    
    lv_obj_t* heatingRateLabel = lv_label_create(thermalSection);
    char heatingText[64];
    float rate50 = summary.getHeatingRateAtTempAndPowerPercent(0, 50.0f);
    snprintf(heatingText, sizeof(heatingText), "Heating Rate (50%%): %.1f°C/s", rate50);
    lv_label_set_text(heatingRateLabel, heatingText);
    
    lv_obj_t* coolingRateLabel = lv_label_create(thermalSection);
    char coolingText[64];
    float coolRate50 = summary.getCoolingRateAtTempAndPowerPercent(0, 50.0f);
    snprintf(coolingText, sizeof(coolingText), "Cooling Rate (50%%): %.1f°C/s", coolRate50);
    lv_label_set_text(coolingRateLabel, coolingText);
    
    // Add door calibration section
    lv_obj_t* doorSection = addMenuSection(summaryPage, "Door Calibration");
    
    lv_obj_t* doorStatus = lv_label_create(doorSection);
    lv_label_set_text(doorStatus, isDoorCalibrated() ? "Status: Calibrated" : "Status: Not Calibrated");
    lv_obj_set_style_text_color(doorStatus, isDoorCalibrated() ? lv_color_hex(0x55FF55) : lv_color_hex(0xFF5555), LV_PART_MAIN);
    
    if (isDoorCalibrated()) {
        lv_obj_t* openPos = lv_label_create(doorSection);
        char openText[32];
        snprintf(openText, sizeof(openText), "Open Position: %.1f°", getDoorOpenPosition());
        lv_label_set_text(openPos, openText);
        
        lv_obj_t* closedPos = lv_label_create(doorSection);
        char closedText[32];
        snprintf(closedText, sizeof(closedText), "Closed Position: %.1f°", getDoorClosedPosition());
        lv_label_set_text(closedPos, closedText);
    }
    
    // Add last calibration time
    lv_obj_t* timeSection = addMenuSection(summaryPage, "Last Calibration");
    
    lv_obj_t* timeLabel = lv_label_create(timeSection);
    char timeText[64];
    uint32_t lastTime = getCalibrationData().lastCalibrationTime;
    if (lastTime > 0) {
        uint32_t hours = lastTime / 3600000;
        uint32_t minutes = (lastTime % 3600000) / 60000;
        snprintf(timeText, sizeof(timeText), "%d hours, %d minutes ago", hours, minutes);
    } else {
        snprintf(timeText, sizeof(timeText), "Never");
    }
    lv_label_set_text(timeLabel, timeText);
    
    // Create back button section
    lv_obj_t* backSection = addMenuSection(summaryPage, "");
    
    lv_obj_t* backBtn = lv_btn_create(backSection);
    lv_obj_set_width(backBtn, LV_PCT(100));
    lv_obj_t* backLabel = lv_label_create(backBtn);
    lv_label_set_text(backLabel, "Back to Calibration Menu");
    lv_obj_center(backLabel);
    lv_obj_add_event_cb(backBtn, [](lv_event_t* e) {
        auto* controller = static_cast<CalibrationController*>(lv_event_get_user_data(e));
        controller->returnToCalibrationMenu();
    }, LV_EVENT_CLICKED, this);
}

void CalibrationController::updateTimerCallback(lv_timer_t* timer) {
    // Cast directly to controller - timer->user_data is set in render() method
    CalibrationController* controller = (CalibrationController*)(timer);
    controller->updateCalibrationProgress();
}

void CalibrationController::updateCalibrationProgress() {
    // Update model from service
    model.updateFromService();
    
    const auto& state = model.getState();
    
    // Update UI based on the current calibration phase
    if (state.phase == CalibrationPhase::TEMPERATURE_CALIBRATION ||
        state.phase == CalibrationPhase::HEATING_CALIBRATION ||
        state.phase == CalibrationPhase::COOLING_CALIBRATION) {
        
        // Update status label
        const char* phaseText = "";
        switch (state.phase) {
            case CalibrationPhase::TEMPERATURE_CALIBRATION:
                phaseText = "Temperature Calibration";
                break;
            case CalibrationPhase::HEATING_CALIBRATION:
                phaseText = "Heating Calibration";
                break;
            case CalibrationPhase::COOLING_CALIBRATION:
                phaseText = "Cooling Calibration";
                break;
            default:
                phaseText = "Unknown";
                break;
        }
        
        if (statusLabel) {
            lv_label_set_text(statusLabel, phaseText);
        }
        
        // Update temperature label
        if (temperatureLabel) {
            char tempText[32];
            snprintf(tempText, sizeof(tempText), "Temperature: %.1f°C", state.currentTemp);
            lv_label_set_text(temperatureLabel, tempText);
        }
        
        // Update progress bar
        if (progressBar) {
            int progress = static_cast<int>(state.progress * 100);
            lv_bar_set_value(progressBar, progress, LV_ANIM_ON);
        }
        
        // Update time remaining label
        if (timeLabel) {
            char timeText[32];
            int seconds = state.timeRemainingMs / 1000;
            snprintf(timeText, sizeof(timeText), "Time remaining: %d sec", seconds);
            lv_label_set_text(timeLabel, timeText);
        }
    }
    
    // Check for errors
    if (state.hasError && state.errorMessage) {
        if (statusLabel) {
            char errorText[64];
            snprintf(errorText, sizeof(errorText), "ERROR: %s", state.errorMessage);
            lv_label_set_text(statusLabel, errorText);
            lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFF0000), LV_PART_MAIN);
        }
    }
}

// Update methods for navigation
void CalibrationController::returnToHome() {
    BuzzerService::getInstance().playMediumTone(300);
    navigateTo("home", 300, TransitionDirection::SLIDE_IN_RIGHT);
}

// Input handlers
void CalibrationController::onEncoderPress() {
    BuzzerService::getInstance().playMediumTone(300);
    // Get focused object and trigger click 
    lv_obj_t* focusedObj = lv_group_get_focused(lv_group_get_default());
    if (focusedObj) {
        // Simulate a click using lv_event_send with proper parameters
        lv_obj_send_event(focusedObj, LV_EVENT_CLICKED, NULL);
    }
}

void CalibrationController::onEncoderLongPress() {
    if (currentView != CalibrationView::MAIN_MENU) {
        // Go back to the main calibration page
        returnToCalibrationMenu();
    } else {
        // Go back to home
        returnToHome();
    }
}

void CalibrationController::onEncoderUp() {
    // Navigate to the previous item in the menu
    lv_group_t* group = lv_group_get_default();
    if (group) {
        lv_group_focus_prev(group);
    }
}

void CalibrationController::onEncoderDown() {
    // Navigate to the next item in the menu
    lv_group_t* group = lv_group_get_default();
    if (group) {
        lv_group_focus_next(group);
    }
}

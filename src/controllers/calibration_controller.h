#pragma once

#include "core/controller.h"
#include "types/calibration_state.h"
#include "types/calibration_data.h"
#include "models/calibration_model.h"
#include "lvgl.h"

struct TemperatureData {
    float current;
    float ambient;
};

// Forward declaration
enum class CalibrationView {
    MAIN_MENU,
    SENSOR_CALIBRATION,
    THERMAL_CALIBRATION,
    DOOR_CALIBRATION,
    SUMMARY
};

class CalibrationController : public Controller {
public:
    static CalibrationController& getInstance();

    void init() override;
    void render() override;

    // Input handlers
    void onEncoderPress() override;
    void onEncoderLongPress() override;
    void onEncoderUp() override;
    void onEncoderDown() override;
    
    // Calibration methods
    void startSensorCalibration();
    void startThermalCalibration();
    void startDoorCalibration();
    void stopCalibration();

    // Getters
    bool isCalibrated() const;
    const CalibrationData& getCalibrationData() const;
    const CalibrationState& getCurrentState() const;
    ThermalCalibrationSummary getThermalSummary() const;
    TemperatureData getCurrentTemperatures() const;
    float getExpectedHeatingRate(float powerPercent) const;
    float getExpectedCoolingRate(float fanPercent) const;
    
    // Navigation
    void viewCalibrationSummary();
    void returnToHome();
    void returnToCalibrationMenu();
    
    // Utility methods
    bool checkTemperatureDifference() const;
    
    // Door calibration methods
    void setDoorOpenPosition(float position);
    void setDoorClosedPosition(float position);
    bool isDoorCalibrated() const;
    float getDoorOpenPosition() const;
    float getDoorClosedPosition() const;
    float getDoorCurrentPosition() const;
    void moveDoorUp();
    void moveDoorDown();

private:
    CalibrationController();
    
    // Set the current view
    void setView(CalibrationView view);
    
    // UI rendering methods
    void renderMenu();
    void renderMainPage();
    void renderSensorCalibrationPage();
    void renderThermalCalibrationPage();
    void renderDoorCalibrationPage();
    void renderSummaryPage();
    
    // UI update methods
    void updateCalibrationProgress();
    void updateDoorCalibrationUI();
    
    // UI helper methods
    lv_obj_t* addMenuSection(lv_obj_t* page, const char* title);
    lv_obj_t* addMenuItem(lv_obj_t* section, const char* text, lv_event_cb_t callback = nullptr);
    
    // UI components
    lv_obj_t* menu = nullptr;
    lv_obj_t* mainPage = nullptr;
    lv_obj_t* sensorCalibrationPage = nullptr;
    lv_obj_t* thermalCalibrationPage = nullptr;
    lv_obj_t* doorCalibrationPage = nullptr;
    lv_obj_t* summaryPage = nullptr;
    
    // Progress indicators
    lv_obj_t* progressBar = nullptr;
    lv_obj_t* progressLabel = nullptr;
    lv_obj_t* temperatureLabel = nullptr;
    lv_obj_t* statusLabel = nullptr;
    lv_obj_t* timeLabel = nullptr;
    
    // Door calibration specific UI
    lv_obj_t* doorPositionLabel = nullptr;
    lv_obj_t* doorStepLabel = nullptr;
    
    // Currently active page
    lv_obj_t* currentPage = nullptr;
    
    // Current view state
    CalibrationView currentView = CalibrationView::MAIN_MENU;
    
    // Data model
    CalibrationModel model;
    
    // Task for updating UI
    lv_timer_t* updateTimer = nullptr;
    static void updateTimerCallback(lv_timer_t* timer);
    
    // Door calibration state
    enum class DoorCalibrationStep {
        OPEN_POSITION,
        CLOSED_POSITION,
        COMPLETE
    } doorStep = DoorCalibrationStep::OPEN_POSITION;
};

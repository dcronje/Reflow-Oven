#include "controllers/calibration_controller.h"
#include "services/calibration_service.h"
#include "services/ui_view_service.h"
#include "services/temperature_control_service.h"
#include "services/sensor_service.h"
#include "ui/views/calibration_summary_view.h"
#include "ui/views/calibration_menu_view.h"
#include "ui/views/thermal_calibration_view.h"
#include "ui/views/sensor_calibration_view.h"
#include "ui/views/sensor_precheck_view.h"
#include <cmath>

CalibrationController& CalibrationController::getInstance() {
    static CalibrationController instance;
    return instance;
}

CalibrationController::CalibrationController() {
    // No setup needed here
}

void CalibrationController::init() {
    CalibrationService::getInstance().init();
    UIViewService::getInstance().registerView("calibration-summary", std::make_unique<CalibrationSummaryView>());
    UIViewService::getInstance().registerView("calibration-menu", std::make_unique<CalibrationMenuView>());
    UIViewService::getInstance().registerView("thermal-calibration", std::make_unique<ThermalCalibrationView>());
    UIViewService::getInstance().registerView("sensor-calibration", std::make_unique<SensorCalibrationView>());
    UIViewService::getInstance().registerView("sensor-precheck", std::make_unique<SensorPrecheckView>());
}

void CalibrationController::startSensorCalibration() {
    CalibrationService::getInstance().startSensorCalibration();
    UIViewService::getInstance().showView("sensor-calibration");
}

void CalibrationController::startThermalCalibration() {
    CalibrationService::getInstance().startThermalCalibration();
    UIViewService::getInstance().showView("thermal-calibration");
}

void CalibrationController::stopCalibration() {
    CalibrationService::getInstance().stopCalibration();
    UIViewService::getInstance().showView("calibration-menu");
}

bool CalibrationController::isCalibrated() const {
    return CalibrationService::getInstance().isCalibrated();
}

const CalibrationData& CalibrationController::getCalibrationData() const {
    return CalibrationService::getInstance().getCalibrationData();
}

const CalibrationState& CalibrationController::getCurrentState() const {
    return CalibrationService::getInstance().getState();
}

ThermalCalibrationSummary CalibrationController::getThermalSummary() const {
    return CalibrationService::getInstance().getCalibrationData().thermalSummary;
}

float CalibrationController::getExpectedHeatingRate(float powerPercent) const {
    return CalibrationService::getInstance().getExpectedHeatingRate(powerPercent);
}

float CalibrationController::getExpectedCoolingRate(float fanPercent) const {
    return CalibrationService::getInstance().getExpectedCoolingRate(fanPercent);
}

void CalibrationController::viewCalibrationSummary() {
    UIViewService::getInstance().showView("calibration-summary");
}

void CalibrationController::returnToMainMenu() {
    UIViewService::getInstance().showView("main-menu");
}

void CalibrationController::returnToCalibrationMenu() {
    UIViewService::getInstance().showView("calibration-menu");
}

bool CalibrationController::checkTemperatureDifference() const {
    auto& tempService = TemperatureControlService::getInstance();
    auto& sensorService = SensorService::getInstance();
    
    float frontTemp = tempService.getFrontTemperature();
    float backTemp = tempService.getBackTemperature();
    float ambientTemp = sensorService.getState().ambientTemp;
    
    return (std::abs(frontTemp - ambientTemp) <= 5.0f) &&
           (std::abs(backTemp - ambientTemp) <= 5.0f);
}

TemperatureData CalibrationController::getCurrentTemperatures() const {
    auto& tempService = TemperatureControlService::getInstance();
    auto& sensorService = SensorService::getInstance();
    
    return {
        .front = tempService.getFrontTemperature(),
        .back = tempService.getBackTemperature(),
        .ambient = sensorService.getState().ambientTemp
    };
}

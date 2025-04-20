#include "controllers/calibration_controller.h"
#include "services/calibration_service.h"
#include "services/ui_view_service.h"
#include "ui/views/calibration_summary_view.h"

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
}

void CalibrationController::startSensorCalibration() {
    CalibrationService::getInstance().startSensorCalibration();
}

void CalibrationController::startThermalCalibration() {
    CalibrationService::getInstance().startThermalCalibration();
}

void CalibrationController::stopCalibration() {
    CalibrationService::getInstance().stopCalibration();
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
    // TODO: Show calibration summary view
    UIViewService::getInstance().showView("calibration-summary");
}

void CalibrationController::returnToMainMenu() {
    UIViewService::getInstance().showView("main-menu");
}

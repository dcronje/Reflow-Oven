#pragma once

#include "types/calibration_state.h"
#include "types/calibration_data.h"

class CalibrationController {
public:
    static CalibrationController& getInstance();

    void init();

    void startSensorCalibration();
    void startThermalCalibration();
    void stopCalibration();

    bool isCalibrated() const;
    const CalibrationData& getCalibrationData() const;
    const CalibrationState& getCurrentState() const;
    ThermalCalibrationSummary getThermalSummary() const;

    float getExpectedHeatingRate(float powerPercent) const;
    float getExpectedCoolingRate(float fanPercent) const;

    void viewCalibrationSummary();
    void returnToMainMenu();

private:
    CalibrationController();
};

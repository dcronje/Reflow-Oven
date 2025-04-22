#pragma once

#include "types/calibration_state.h"
#include "types/calibration_data.h"

struct TemperatureData {
    float front;
    float back;
    float ambient;
};

class CalibrationController {
public:
    static CalibrationController& getInstance();

    void init();

    void startSensorCalibration();
    void startThermalCalibration();
    void startDoorCalibration();
    void stopCalibration();

    bool isCalibrated() const;
    const CalibrationData& getCalibrationData() const;
    const CalibrationState& getCurrentState() const;
    ThermalCalibrationSummary getThermalSummary() const;
    TemperatureData getCurrentTemperatures() const;

    float getExpectedHeatingRate(float powerPercent) const;
    float getExpectedCoolingRate(float fanPercent) const;

    void viewCalibrationSummary();
    void returnToMainMenu();
    void returnToCalibrationMenu();
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
};

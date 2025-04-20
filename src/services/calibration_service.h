#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "types/calibration_data.h"
#include "types/calibration_state.h"

class CalibrationService {
public:
    static CalibrationService& getInstance();

    void init();
    void startSensorCalibration();
    void startThermalCalibration();
    void stopCalibration();

    bool isCalibrated() const;
    const CalibrationData& getCalibrationData() const;
    const CalibrationState& getState() const;

    float getExpectedHeatingRate(float powerPercent) const;
    float getExpectedCoolingRate(float fanPercent) const;

private:
    CalibrationService();

    static void calibrationTaskWrapper(void* pvParameters);
    void calibrationTask();

    bool runSensorCalibration();
    bool runThermalCalibration();
    bool saveCalibrationData();
    bool loadCalibrationData();

    void updateProgress(const char* label, float progress, float currentTemp, uint32_t remaining);
    void displayError(const char* message);

    // Calibration parameters
    static const uint32_t TEMP_CALIBRATION_TIME_MS = 30000;     // 30 seconds
    static const uint32_t THERMAL_CALIBRATION_TIME_MS = 60000;  // 60 seconds per power level
    static const uint32_t COOLING_TEST_TIME_MS = 120000;        // 2 minutes per test
    static constexpr float MIN_TEMP_DIFF_FOR_WARNING = 5.0f;    // 5°C difference triggers warning

    CalibrationData data;
    CalibrationState state;
    TaskHandle_t taskHandle;
    QueueHandle_t updateQueue;

    enum class Mode {
        NONE,
        SENSOR,
        THERMAL
    } currentMode;

    absolute_time_t calibrationStartTime;
};
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
    void startDoorCalibration();
    void stopCalibration();

    bool isCalibrated() const;
    const CalibrationData& getCalibrationData() const;
    const CalibrationState& getState() const;

    float getExpectedHeatingRate(float powerPercent) const;
    float getExpectedCoolingRate(float fanPercent) const;

    // Door calibration methods
    void setDoorOpenPosition(float position);
    void setDoorClosedPosition(float position);
    bool isDoorCalibrated() const;
    float getDoorOpenPosition() const;
    float getDoorClosedPosition() const;

private:
    CalibrationService();

    static void calibrationTaskWrapper(void* pvParameters);
    void calibrationTask();

    bool runSensorCalibration();
    bool runThermalCalibration();
    bool runDoorCalibration();
    bool saveCalibrationData();
    bool loadCalibrationData();

    void updateProgress(const char* label, float progress, float currentTemp, uint32_t remaining);
    void displayError(const char* message);

    // Calibration parameters
    static const uint32_t TEMP_CALIBRATION_TIME_MS = 30000;     // 30 seconds
    static const uint32_t THERMAL_CALIBRATION_TIME_MS = 60000;  // 60 seconds per power level
    static const uint32_t COOLING_TEST_TIME_MS = 120000;        // 2 minutes per test
    static const uint32_t THERMAL_SETTLE_TIME_MS = 10000;       // 10 seconds settle time
    static constexpr float MIN_TEMP_DIFF_FOR_WARNING = 5.0f;    // 5°C difference triggers warning

    // Temperature points for multi-point calibration
    static constexpr float TEMP_POINTS[] = {25.0f, 100.0f, 200.0f};  // °C
    static constexpr size_t NUM_TEMP_POINTS = sizeof(TEMP_POINTS) / sizeof(TEMP_POINTS[0]);

    CalibrationData data;
    CalibrationState state;
    TaskHandle_t taskHandle;
    QueueHandle_t updateQueue;

    enum class Mode {
        NONE,
        SENSOR,
        THERMAL,
        DOOR
    } currentMode;

    absolute_time_t calibrationStartTime;
};
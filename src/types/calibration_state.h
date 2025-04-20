#pragma once

#include <stdint.h>

enum class CalibrationPhase {
    IDLE,
    TEMPERATURE_CALIBRATION,
    HEATING_CALIBRATION,
    COOLING_CALIBRATION,
    COMPLETE,
    ERROR
};

struct CalibrationState {
    CalibrationPhase phase;
    float progress;             // 0.0 to 1.0
    float currentTemp;          // Current temperature during calibration
    uint32_t timeRemainingMs;   // Estimated time left in ms
    bool hasError;
    const char* errorMessage;   // Pointer to static or persistent string
};
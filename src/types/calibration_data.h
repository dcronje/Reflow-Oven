#pragma once
#include <stdint.h>

struct ThermalCalibrationSummary {
    float heatingRates[10];     // Index 0 = 10%, 1 = 20%, ..., 9 = 100%
    float coolingRates[10];     // Index 0 = 10%, 1 = 20%, ..., 9 = 100%
};

struct DoorCalibrationData {
    bool isCalibrated = false;
    float openPosition = 0.0f;    // Position in degrees when door is fully open
    float closedPosition = 0.0f;  // Position in degrees when door is fully closed
};

struct CalibrationData {
    float frontSensorOffset;
    float backSensorOffset;
    ThermalCalibrationSummary thermalSummary;
    DoorCalibrationData doorCalibration;
    uint32_t lastCalibrationTime;
    bool isCalibrated;
};
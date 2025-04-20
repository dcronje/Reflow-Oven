#pragma once
#include <stdint.h>

struct ThermalCalibrationSummary {
    float heatingRates[10];     // Index 0 = 10%, 1 = 20%, ..., 9 = 100%
    float coolingRates[10];     // Index 0 = 10%, 1 = 20%, ..., 9 = 100%
};

struct CalibrationData {
    float frontSensorOffset;
    float backSensorOffset;
    ThermalCalibrationSummary thermalSummary;
    uint32_t lastCalibrationTime;
    bool isCalibrated;
};
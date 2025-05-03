#pragma once

#include <stdint.h>
#include <array>

struct ThermalCalibrationSummary {
    // Rates at different temperatures [temp_point][power_level]
    float heatingRates[3][10];     // 3 temperature points, 10 power levels (10% to 100%)
    float coolingRates[3][10];     // 3 temperature points, 10 fan levels (10% to 100%)

    // Helper methods to get rates for a specific temperature point
    std::array<float, 10> getHeatingRatesAtTemp(int tempIdx) const {
        std::array<float, 10> rates;
        for (int i = 0; i < 10; i++) {
            rates[i] = heatingRates[tempIdx][i];
        }
        return rates;
    }

    std::array<float, 10> getCoolingRatesAtTemp(int tempIdx) const {
        std::array<float, 10> rates;
        for (int i = 0; i < 10; i++) {
            rates[i] = coolingRates[tempIdx][i];
        }
        return rates;
    }

    // Helper methods to get a specific rate
    float getHeatingRateAtTempAndPower(int tempIdx, int powerIdx) const { return heatingRates[tempIdx][powerIdx]; }
    float getCoolingRateAtTempAndPower(int tempIdx, int powerIdx) const { return coolingRates[tempIdx][powerIdx]; }

    // Helper methods to get a specific rate by power percentage
    float getHeatingRateAtTempAndPowerPercent(int tempIdx, float powerPercent) const {
        int idx = static_cast<int>(powerPercent / 10.0f) - 1;
        if (idx < 0) idx = 0;
        if (idx > 9) idx = 9;
        return heatingRates[tempIdx][idx];
    }

    float getCoolingRateAtTempAndPowerPercent(int tempIdx, float powerPercent) const {
        int idx = static_cast<int>(powerPercent / 10.0f) - 1;
        if (idx < 0) idx = 0;
        if (idx > 9) idx = 9;
        return coolingRates[tempIdx][idx];
    }
};

struct DoorCalibrationData {
    bool isCalibrated = false;
    float openPosition = 0.0f;    // Position in degrees when door is fully open
    float closedPosition = 0.0f;  // Position in degrees when door is fully closed
};

struct CalibrationData {
    float sensorOffset;
    ThermalCalibrationSummary thermalSummary;
    DoorCalibrationData doorCalibration;
    uint32_t lastCalibrationTime;
    bool isCalibrated;
};
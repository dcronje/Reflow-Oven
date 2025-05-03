#pragma once

#include <cstdint>

struct TemperatureState {
    float currentTemp;
    float targetTemp;

    float output;

    bool isHeating;
    bool isCooling;

    uint8_t coolingPower;
    uint16_t fanRPM;

    bool hasError;
    const char* lastError;
};

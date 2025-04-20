#pragma once

#include <cstdint>

struct TemperatureState {
    float frontTemp;
    float backTemp;
    float targetTemp;

    float frontOutput;
    float backOutput;

    bool isHeating;
    bool isCooling;

    uint8_t coolingPower;
    uint16_t fanRPM;

    bool hasError;
    const char* lastError;
};

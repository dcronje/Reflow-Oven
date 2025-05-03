#pragma once

#include <cstdint>
#include <string>

struct SensorState {
    float currentTemp = 0.0f;
    float ambientTemp = 0.0f;
    float ambientHumidity = 0.0f;
    float ssrTemp = 0.0f;
    bool hasError = false;
    std::string lastError;
};
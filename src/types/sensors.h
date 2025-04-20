#pragma once

#include <cstdint>
#include <string>

struct SensorState {
    float frontTemp = 0.0f;
    float backTemp = 0.0f;
    float ambientTemp = 0.0f;
    float ambientHumidity = 0.0f;
    bool hasError = false;
    std::string lastError;
};
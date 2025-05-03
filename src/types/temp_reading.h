#pragma once

#include <stdint.h>

struct TempReading {
    float currentTemp; 
    uint32_t timestamp; // ms since boot
};

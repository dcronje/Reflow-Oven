#pragma once

#include <stdint.h>

struct TempReading {
    float frontTemp;
    float backTemp;
    uint32_t timestamp; // ms since boot
};

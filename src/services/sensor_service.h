#pragma once

#include "library/sht30.h"
#include "types/sensors.h"
#include "pico/types.h"
#include <string>

class SensorService {
public:
    static SensorService& getInstance();

    void init();
    const SensorState& getState() const;

private:
    SensorService();
    void sensorTask();

    SensorState state;
    SHT30 sht30;
};

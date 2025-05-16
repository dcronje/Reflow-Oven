#pragma once

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "constants.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


class ElectronicsCoolingService {
public:
    static ElectronicsCoolingService& getInstance();

    void init();

private:
    ElectronicsCoolingService();
    void electronicsCoolingTask();
    uint calculatePWMWrapValue(uint frequency);

    volatile int currentFanSpeed = 0;
    volatile int targetFanSpeed = 0;
}; 
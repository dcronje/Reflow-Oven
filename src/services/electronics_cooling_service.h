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
    static void rpmTimerCallback(TimerHandle_t xTimer);
    uint calculatePWMWrapValue(uint frequency);

    TimerHandle_t rpmTimer;

    volatile int currentFanSpeed = 0;
    volatile int targetFanSpeed = 0;
    volatile uint64_t fanRPM = 0;
    volatile uint64_t pulseCount = 0;
}; 
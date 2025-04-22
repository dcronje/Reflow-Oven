#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "servo.pio.h"
#include "constants.h"
#include "FreeRTOS.h"
#include "task.h"

struct ServoConfig {
    uint minPulse;  // Pulse width for 0 degrees
    uint maxPulse;  // Pulse width for 180 degrees
    bool invert;    // Whether to invert the angle
};

enum class DoorDirection {
    NONE,
    OPENING,
    CLOSING
};

class DoorService {
public:
    static DoorService& getInstance();

    void init();
    void setPosition(uint8_t percent);
    void setCalibrationAngles(uint8_t closedAngle, uint8_t openAngle);
    bool isFullyOpen() const;
    bool isFullyClosed() const;

    // For calibration use
    void setRawAngle(uint8_t angle);
    uint8_t getCurrentAngle() const;

    // Safety control
    void enableServo();
    void disableServo();
    bool isServoEnabled() const;
    bool isSafeToMove() const;

private:
    DoorService();
    void initServoSm(PIO pio, uint sm, uint offset, float clkDiv, uint pin, uint32_t periodTicks);
    void setServoAngle(uint8_t angle);
    static void safetyMonitorTask(void* pvParameters);
    void safetyMonitor();

    uint doorSm;
    ServoConfig doorConfig;
    uint8_t doorClosedAngle;
    uint8_t doorOpenAngle;
    uint8_t currentAngle;
    uint8_t targetAngle;
    bool servoEnabled;
    TaskHandle_t safetyTaskHandle;
    DoorDirection direction;
}; 
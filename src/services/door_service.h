#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "servo.pio.h"
#include "constants.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/adc.h"

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
    void enableServo();
    void disableServo();
    bool isEnabled() const;

    // Position control (0-100%)
    void setPosition(uint8_t percent);
    uint8_t getPosition() const;
    float getCurrentAngle() const;  // Returns actual angle in degrees

    // Limit switches
    bool isFullyOpen() const;
    bool isFullyClosed() const;

    // Raw control for calibration
    void setRawAngle(uint8_t angle);  // 0-270 degrees
    uint8_t getCurrentRawAngle() const;  // Get actual angle from feedback

    // For calibration use
    void setCalibrationAngles(uint8_t closedAngle, uint8_t openAngle);

    // Safety control
    bool isServoEnabled() const;
    bool isSafeToMove() const;

private:
    DoorService();
    void initServoSm(PIO pio, uint sm, uint offset, float clkDiv, uint pin, uint32_t periodTicks);
    void setServoAngle(uint8_t angle);
    static void safetyMonitorTask(void* pvParameters);
    void safetyMonitor();
    static void doorTaskWrapper(void* pvParameters);
    void doorTask();
    void updateServoPosition();
    void readFeedback();
    void protectPins(bool protect);

    uint doorSm;
    ServoConfig doorConfig;
    uint8_t doorClosedAngle;
    uint8_t doorOpenAngle;
    uint8_t currentAngle;
    uint8_t targetAngle;
    bool servoEnabled;
    TaskHandle_t safetyTaskHandle;
    DoorDirection direction;

    bool enabled = false;
    uint8_t targetPosition = 0;  // 0-100%
    uint8_t feedbackValue = 0;   // Raw ADC reading

    // Pin definitions
    static constexpr uint8_t SERVO_PWM_PIN = DOOR_SERVO_CONTROL_GPIO;
    static constexpr uint8_t SERVO_POWER_PIN = SERVO_POWER_GPIO;
    static constexpr uint8_t SERVO_FEEDBACK_PIN = DOOR_SERVO_FEEDBACK_GPIO;  // ADC0
    static constexpr uint8_t OPEN_SWITCH_PIN = DOOR_OPEN_SWITCH_GPIO;
    static constexpr uint8_t CLOSED_SWITCH_PIN = DOOR_CLOSED_SWITCH_GPIO;

    // PWM parameters for 270-degree servo
    static constexpr uint32_t SERVO_MIN_PULSE = 500;   // ~0.5ms for 0 degrees
    static constexpr uint32_t SERVO_MAX_PULSE = 2500;  // ~2.5ms for 270 degrees
    static constexpr uint32_t SERVO_PERIOD = 20000;    // 20ms period

    QueueHandle_t commandQueue;
    TaskHandle_t taskHandle;

    struct DoorCommand {
        uint8_t position;
    };
}; 
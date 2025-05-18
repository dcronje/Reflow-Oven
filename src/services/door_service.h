#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "servo.pio.h"
#include "constants.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "hardware/adc.h"
#include <functional>
#include <vector>

// Define event bits for the door event group
#define DOOR_EVENT_OPENING      (1 << 0)
#define DOOR_EVENT_OPENED       (1 << 1)
#define DOOR_EVENT_CLOSING      (1 << 2)
#define DOOR_EVENT_CLOSED       (1 << 3)
#define DOOR_EVENT_ENABLED      (1 << 4)
#define DOOR_EVENT_DISABLED     (1 << 5)
#define DOOR_EVENT_POSITION     (1 << 6)

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

// Door event types
enum class DoorEventType {
    DOOR_OPENING,    // Door has started opening
    DOOR_OPENED,     // Door has fully opened
    DOOR_CLOSING,    // Door has started closing
    DOOR_CLOSED,     // Door has fully closed
    DOOR_POSITION,   // Door position has changed (includes percentage)
    DOOR_ENABLED,    // Door servo has been enabled
    DOOR_DISABLED    // Door servo has been disabled
};

// Event data structure
struct DoorEvent {
    DoorEventType type;
    int positionPercent; // 0-100% open
};

// Callback function type
typedef std::function<void(const DoorEvent&)> DoorEventCallback;

class DoorService {
public:
    static DoorService& getInstance();

    void init();
    void enableServo();
    void disableServo();
    bool isEnabled() const;

    // High-level door operations
    void open();  // Open the door fully
    void close(); // Close the door fully
    
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
    
    // Get current door direction
    DoorDirection getDoorDirection() const { return direction; }
    
    // Event handling
    void addEventListener(DoorEventCallback callback);
    void removeEventListener(DoorEventCallback callback);
    
    // Event Group access - for FreeRTOS Event Group system
    EventGroupHandle_t getDoorEventGroup() const { return doorEventGroup; }
    uint8_t getLastPositionEvent() const { return lastPositionPercent; }

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
    
    // Event methods
    void emitEvent(DoorEventType type, int positionPercent);
    void checkAndEmitLimitEvents();
    
    // Event group method (for FreeRTOS events)
    void setEventGroupBits(EventBits_t bits, int positionPercent);

    // Helper methods
    static uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max);

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
    
    // Event handling
    std::vector<DoorEventCallback> eventListeners;
    bool wasFullyOpen = false;
    bool wasFullyClosed = false;
    
    // Event group for FreeRTOS based messaging
    EventGroupHandle_t doorEventGroup;
    uint8_t lastPositionPercent;

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
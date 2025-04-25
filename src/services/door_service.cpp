#include "services/door_service.h"
#include "constants.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "pico/time.h"

DoorService& DoorService::getInstance() {
    static DoorService instance;
    return instance;
}

DoorService::DoorService()
    : doorSm(0),
      doorConfig{1950, 3900, false},  // Default 0-180 degrees
      doorClosedAngle(0),
      doorOpenAngle(180),
      currentAngle(0),
      targetAngle(0),
      servoEnabled(false),
      safetyTaskHandle(nullptr),
      direction(DoorDirection::NONE) {
}

void DoorService::init() {
    // Initialize MOSFET control for servo power
    gpio_init(SERVO_POWER_GPIO);
    gpio_set_dir(SERVO_POWER_GPIO, GPIO_OUT);
    gpio_put(SERVO_POWER_GPIO, 0); // Start with servo disabled

    // Initialize door switches
    gpio_init(DOOR_OPEN_SWITCH_GPIO);
    gpio_set_dir(DOOR_OPEN_SWITCH_GPIO, GPIO_IN);

    gpio_init(DOOR_CLOSED_SWITCH_GPIO);
    gpio_set_dir(DOOR_CLOSED_SWITCH_GPIO, GPIO_IN);

    // Initialize door servo
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &servo_program);

    doorSm = pio_claim_unused_sm(pio, true);
    initServoSm(pio, doorSm, offset, 64.0f, DOOR_SERVO_GPIO, SERVO_PERIOD_TICKS);
    pio_sm_set_enabled(pio, doorSm, true);

    // Initialize to closed position
    setPosition(0);

    // Start safety monitoring task
    xTaskCreate(safetyMonitorTask, "DoorSafety", 256, this, 2, &safetyTaskHandle);
}

void DoorService::enableServo() {
    if (isSafeToMove()) {
        gpio_put(SERVO_POWER_GPIO, 1);
        servoEnabled = true;
    }
}

void DoorService::disableServo() {
    gpio_put(SERVO_POWER_GPIO, 0);
    servoEnabled = false;
}

bool DoorService::isServoEnabled() const {
    return servoEnabled;
}

bool DoorService::isSafeToMove() const {
    // If door is not moving, it's safe to start moving
    if (direction == DoorDirection::NONE) {
        return true;
    }

    // Check if we're moving towards a limit that's already triggered
    if (direction == DoorDirection::CLOSING && isFullyClosed()) {
        return false;
    }
    if (direction == DoorDirection::OPENING && isFullyOpen()) {
        return false;
    }

    return true;
}

void DoorService::safetyMonitorTask(void* pvParameters) {
    static_cast<DoorService*>(pvParameters)->safetyMonitor();
}

void DoorService::safetyMonitor() {
    const TickType_t xDelay = pdMS_TO_TICKS(100); // Check every 100ms
    
    while (true) {
        if (servoEnabled) {
            if (!isSafeToMove()) {
                // Emergency stop if unsafe
                disableServo();
            }
        }
        vTaskDelay(xDelay);
    }
}

void DoorService::setPosition(uint8_t percent) {
    if (!servoEnabled) {
        return; // Ignore position changes if servo is disabled
    }

    // Calculate target angle
    targetAngle = map(percent, 0, 100, doorClosedAngle, doorOpenAngle);
    
    // Determine direction
    if (targetAngle > currentAngle) {
        direction = DoorDirection::OPENING;
    } else if (targetAngle < currentAngle) {
        direction = DoorDirection::CLOSING;
    } else {
        direction = DoorDirection::NONE;
    }

    // Check if safe to move
    if (!isSafeToMove()) {
        disableServo();
        return;
    }

    setRawAngle(targetAngle);
}

void DoorService::setCalibrationAngles(uint8_t closedAngle, uint8_t openAngle) {
    doorClosedAngle = closedAngle;
    doorOpenAngle = openAngle;
}

void DoorService::setRawAngle(uint8_t angle) {
    setServoAngle(angle);
    currentAngle = angle;
    
    // Update direction based on new position
    if (currentAngle == targetAngle) {
        direction = DoorDirection::NONE;
    }
}

uint8_t DoorService::getCurrentAngle() const {
    return currentAngle;
}

void DoorService::setServoAngle(uint8_t angle) {
    if (doorConfig.invert) {
        angle = 180 - angle;
    }
    
    uint pulse = doorConfig.minPulse + ((doorConfig.maxPulse - doorConfig.minPulse) * angle) / 180;
    pio_sm_put_blocking(pio0, doorSm, pulse);
}

bool DoorService::isFullyOpen() const {
    return !gpio_get(DOOR_OPEN_SWITCH_GPIO);
}

bool DoorService::isFullyClosed() const {
    return !gpio_get(DOOR_CLOSED_SWITCH_GPIO);
}

void DoorService::initServoSm(PIO pio, uint sm, uint offset, float clkDiv, uint pin, uint32_t periodTicks) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_exec(pio, sm, pio_encode_set(pio_isr, periodTicks));
    pio_sm_config c = servo_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_clkdiv(&c, clkDiv);
    pio_sm_init(pio, sm, offset, &c);
} 
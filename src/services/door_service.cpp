#include "services/door_service.h"
#include "constants.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
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
      direction(DoorDirection::NONE),
      commandQueue(xQueueCreate(10, sizeof(DoorCommand))),
      taskHandle(nullptr) {
}

void DoorService::init() {
    // Configure power control pin
    gpio_set_function(SERVO_POWER_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(SERVO_POWER_PIN, GPIO_OUT);
    gpio_put(SERVO_POWER_PIN, 0);  // Start with servo disabled

    // Configure limit switches
    gpio_set_function(OPEN_SWITCH_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(OPEN_SWITCH_PIN, GPIO_IN);

    gpio_set_function(CLOSED_SWITCH_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(CLOSED_SWITCH_PIN, GPIO_IN);

    // Configure ADC for feedback
    adc_init();
    adc_gpio_init(SERVO_FEEDBACK_PIN);
    adc_select_input(0);  // ADC0

    // Initialize door servo
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &servo_program);

    doorSm = pio_claim_unused_sm(pio, true);
    initServoSm(pio, doorSm, offset, 64.0f, DOOR_SERVO_CONTROL_GPIO, SERVO_PERIOD);
    pio_sm_set_enabled(pio, doorSm, true);

    // Initialize to closed position
    setPosition(0);

    // Start safety monitoring task
    xTaskCreate(safetyMonitorTask, "DoorSafety", 256, this, 2, &safetyTaskHandle);

    // Create door control task
    xTaskCreate(doorTaskWrapper, "DoorTask", 256, this, 1, &taskHandle);
}

void DoorService::enableServo() {
    if (isSafeToMove()) {
        // Power on the servo
        gpio_put(SERVO_POWER_PIN, 1);
        servoEnabled = true;

        // Enable pins after power is stable
        protectPins(false);
    }
}

void DoorService::disableServo() {
    // Protect pins before power off
    protectPins(true);

    // Disable power to the servo via the MOSFET
    gpio_put(SERVO_POWER_PIN, 0);
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

float DoorService::getCurrentAngle() const {
    return (currentAngle * 270.0f) / 255.0f;  // Convert 0-255 to 0-270 degrees
}

void DoorService::setServoAngle(uint8_t angle) {
    if (doorConfig.invert) {
        angle = 180 - angle;
    }
    
    uint pulse = doorConfig.minPulse + ((doorConfig.maxPulse - doorConfig.minPulse) * angle) / 180;
    pio_sm_put_blocking(pio0, doorSm, pulse);
}

bool DoorService::isFullyOpen() const {
    return !gpio_get(OPEN_SWITCH_PIN);  // Active low
}

bool DoorService::isFullyClosed() const {
    return !gpio_get(CLOSED_SWITCH_PIN);  // Active low
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

void DoorService::protectPins(bool protect) {
    if (protect) {
        // Disable PIO output and set servo control pin to high-impedance input
        pio_sm_set_enabled(pio0, doorSm, false);
        gpio_set_function(DOOR_SERVO_CONTROL_GPIO, GPIO_FUNC_SIO);
        gpio_set_dir(DOOR_SERVO_CONTROL_GPIO, GPIO_IN);
        gpio_disable_pulls(DOOR_SERVO_CONTROL_GPIO);

        // Set feedback pin to high-impedance input
        gpio_set_function(SERVO_FEEDBACK_PIN, GPIO_FUNC_SIO);
        gpio_set_dir(SERVO_FEEDBACK_PIN, GPIO_IN);
        gpio_disable_pulls(SERVO_FEEDBACK_PIN);
    } else {
        // Restore PIO functionality for servo control
        gpio_set_function(DOOR_SERVO_CONTROL_GPIO, GPIO_FUNC_PIO0);
        pio_sm_set_enabled(pio0, doorSm, true);

        // Restore ADC functionality for feedback
        gpio_set_function(SERVO_FEEDBACK_PIN, GPIO_FUNC_SIO);
        adc_gpio_init(SERVO_FEEDBACK_PIN);
        adc_select_input(0);  // ADC0
    }
}

void DoorService::readFeedback() {
    // Read ADC value (0-4095)
    uint16_t raw = adc_read();
    
    // Convert to 0-255 range for angle
    feedbackValue = (raw * 255) / 4095;
    
    // Update current angle
    currentAngle = feedbackValue;
}

void DoorService::doorTaskWrapper(void* pvParameters) {
    static_cast<DoorService*>(pvParameters)->doorTask();
}

void DoorService::doorTask() {
    DoorCommand cmd;
    while (true) {
        if (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            targetAngle = cmd.position;
            updateServoPosition();
        }
        
        if (servoEnabled) {
            readFeedback();
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));  // 50Hz update rate
    }
}

void DoorService::updateServoPosition() {
    if (!servoEnabled) return;
    
    // Convert percentage to angle (0-270 degrees)
    uint8_t angle = (targetAngle * 270) / 100;
    setRawAngle(angle);
}

uint8_t DoorService::getCurrentRawAngle() const {
    return currentAngle;
} 
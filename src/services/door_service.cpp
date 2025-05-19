#include "services/door_service.h"
#include "core/message_event_bus.h"
#include "core/message_event_adapter.h"
#include "constants.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/time.h"
#include <algorithm> // For std::remove_if
#include "pb_encode.h"
#include "pb_decode.h"

// Include generated proto files
#include "generated/protos/common.pb.h"
#include "generated/protos/hardware.pb.h"

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
      taskHandle(nullptr),
      doorEventGroup(xEventGroupCreate()),
      lastPositionPercent(0) {
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
        
        // Emit event using new message system
        publishDoorEvent(DoorEventType::DOOR_ENABLED, getPosition());
    }
}

void DoorService::disableServo() {
    // Protect pins before power off
    protectPins(true);

    // Disable power to the servo via the MOSFET
    gpio_put(SERVO_POWER_PIN, 0);
    servoEnabled = false;
    
    // Emit event using new message system
    publishDoorEvent(DoorEventType::DOOR_DISABLED, getPosition());
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
            // Check for limit switches and emit events if needed
            checkAndEmitLimitEvents();
            
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
    
    // Store the target position
    targetPosition = percent;

    // Calculate target angle
    targetAngle = map(percent, 0, 100, doorClosedAngle, doorOpenAngle);
    
    // Determine direction and emit appropriate event
    if (targetAngle > currentAngle) {
        direction = DoorDirection::OPENING;
        publishDoorEvent(DoorEventType::DOOR_OPENING, percent);
    } else if (targetAngle < currentAngle) {
        direction = DoorDirection::CLOSING;
        publishDoorEvent(DoorEventType::DOOR_CLOSING, percent);
    } else {
        direction = DoorDirection::NONE;
    }

    // Check if safe to move
    if (!isSafeToMove()) {
        disableServo();
        return;
    }

    setRawAngle(targetAngle);
    
    // Emit position change event
    publishDoorEvent(DoorEventType::DOOR_POSITION, percent);
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
    
    // Periodically publish status (every ~1 second)
    static uint32_t lastPublishTime = 0;
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    
    if (currentTime - lastPublishTime > 1000) {
        publishDoorStatus(getPosition());
        lastPublishTime = currentTime;
    }
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

// Legacy event handling implementations

void DoorService::addEventListener(DoorEventCallback callback) {
    // Add callback to listeners vector
    eventListeners.push_back(callback);
}

void DoorService::removeEventListener(DoorEventCallback callback) {
    // Since we can't directly compare std::function objects for equality,
    // this is a placeholder. In a real implementation, you'd need a way
    // to identify listeners, such as using a token or ID.
    // This is just to demonstrate the concept.
    // eventListeners.erase(std::remove_if(eventListeners.begin(), eventListeners.end(), 
    //    [&callback](const DoorEventCallback& c) { return &c == &callback; }), 
    //    eventListeners.end());
}

void DoorService::emitEvent(DoorEventType type, int positionPercent) {
    // Create event object for legacy system
    DoorEvent doorEvent{type, positionPercent};
    
    // Set corresponding event group bits for FreeRTOS Events
    EventBits_t bits = 0;
    
    switch (type) {
        case DoorEventType::DOOR_OPENING:
            bits = DOOR_EVENT_OPENING;
            break;
        case DoorEventType::DOOR_OPENED:
            bits = DOOR_EVENT_OPENED;
            break;
        case DoorEventType::DOOR_CLOSING:
            bits = DOOR_EVENT_CLOSING;
            break;
        case DoorEventType::DOOR_CLOSED:
            bits = DOOR_EVENT_CLOSED;
            break;
        case DoorEventType::DOOR_ENABLED:
            bits = DOOR_EVENT_ENABLED;
            break;
        case DoorEventType::DOOR_DISABLED:
            bits = DOOR_EVENT_DISABLED;
            break;
        case DoorEventType::DOOR_POSITION:
            bits = DOOR_EVENT_POSITION;
            break;
    }
    
    // Set the event group bits if applicable
    if (bits != 0) {
        // Store position for position events
        if (bits & DOOR_EVENT_POSITION) {
            lastPositionPercent = positionPercent;
        }
        
        // Set the bits in the event group
        xEventGroupSetBits(doorEventGroup, bits);
    }
    
    // Notify all legacy listeners
    for (const auto& listener : eventListeners) {
        listener(doorEvent);
    }
}

void DoorService::checkAndEmitLimitEvents() {
    // Check if door has just opened fully
    bool isOpen = isFullyOpen();
    if (isOpen && !wasFullyOpen) {
        publishDoorEvent(DoorEventType::DOOR_OPENED, 100);
    }
    wasFullyOpen = isOpen;
    
    // Check if door has just closed fully
    bool isClosed = isFullyClosed();
    if (isClosed && !wasFullyClosed) {
        publishDoorEvent(DoorEventType::DOOR_CLOSED, 0);
    }
    wasFullyClosed = isClosed;
}

// Add this method to get current position as percentage
uint8_t DoorService::getPosition() const {
    // Calculate position percentage based on current angle
    return map(currentAngle, doorClosedAngle, doorOpenAngle, 0, 100);
}

// Add this helper method for mapping values
uint8_t DoorService::map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool DoorService::isEnabled() const {
    return servoEnabled;
}

void DoorService::open() {
    // Open the door to 100% position
    setPosition(100);
}

void DoorService::close() {
    // Close the door to 0% position
    setPosition(0);
}

// Implementation for MessageHandler
void DoorService::processMessage(const void* data, size_t size) {
    // Try to decode the message as a hardware message
    reflow_HardwareMessage message = reflow_HardwareMessage_init_zero;
    
    // Create a stream for decoding
    pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t*)data, size);
    
    // Decode the message
    bool status = pb_decode(&stream, reflow_HardwareMessage_fields, &message);
    if (!status) {
        // Failed to decode
        return;
    }
    
    // Check if this is a door command
    if (message.which_payload != reflow_HardwareMessage_door_command_tag) {
        // Not a door command, ignore
        return;
    }
    
    // Process the door command
    const reflow_DoorCommand& command = message.payload.door_command;
    
    switch (command.command) {
        case reflow_DoorCommand_CommandType_OPEN:
            open();
            break;
            
        case reflow_DoorCommand_CommandType_CLOSE:
            close();
            break;
            
        case reflow_DoorCommand_CommandType_SET_POSITION:
            setPosition(command.position);
            break;
            
        case reflow_DoorCommand_CommandType_ENABLE:
            enableServo();
            break;
            
        case reflow_DoorCommand_CommandType_DISABLE:
            disableServo();
            break;
            
        case reflow_DoorCommand_CommandType_CALIBRATE:
            // Custom calibration logic would go here
            break;
            
        default:
            // Unknown command
            break;
    }
}

void DoorService::processMessage(const std::string& serialized) {
    // Process serialized message by converting to binary first
    processMessage(serialized.data(), serialized.size());
}

void DoorService::publishDoorStatus(uint8_t position) {
    // Create a hardware message to wrap the door event
    reflow_HardwareMessage message = reflow_HardwareMessage_init_zero;
    
    // Initialize the base message
    message.has_base = true;
    
    // Set base message fields
    message.base.sequence_id = MessageEventAdapter::getInstance().getNextMessageId();
    message.base.timestamp = MessageEventAdapter::getInstance().getCurrentTimestamp();
    message.base.priority = 1; // Normal priority
    message.base.type = reflow_BaseMessage_MessageType_EVENT;
    
    // Set up the door event with status information
    message.which_payload = reflow_HardwareMessage_door_event_tag;
    message.payload.door_event.event = reflow_DoorEvent_EventType_POSITION_CHANGED;
    message.payload.door_event.position = position;
    message.payload.door_event.is_moving = direction != DoorDirection::NONE;
    
    // Serialize the message
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool status = pb_encode(&stream, reflow_HardwareMessage_fields, &message);
    
    if (status) {
        // Publish via the message event adapter
        MessageEventAdapter::getInstance().postMessage(buffer, stream.bytes_written);
    }
    
    // Also emit legacy event for backward compatibility
    emitEvent(DoorEventType::DOOR_POSITION, position);
}

void DoorService::publishDoorEvent(DoorEventType type, int positionPercent) {
    // Create a hardware message to wrap the door event
    reflow_HardwareMessage message = reflow_HardwareMessage_init_zero;
    
    // Initialize the base message
    message.has_base = true;
    
    // Set base message fields
    message.base.sequence_id = MessageEventAdapter::getInstance().getNextMessageId();
    message.base.timestamp = MessageEventAdapter::getInstance().getCurrentTimestamp();
    message.base.priority = 1; // Normal priority
    message.base.type = reflow_BaseMessage_MessageType_EVENT;
    
    // Set up the door event
    message.which_payload = reflow_HardwareMessage_door_event_tag;
    message.payload.door_event.position = positionPercent;
    message.payload.door_event.is_moving = direction != DoorDirection::NONE;
    
    // Map the event type
    switch (type) {
        case DoorEventType::DOOR_OPENING:
            message.payload.door_event.event = reflow_DoorEvent_EventType_OPENING;
            break;
        case DoorEventType::DOOR_OPENED:
            message.payload.door_event.event = reflow_DoorEvent_EventType_OPENED;
            break;
        case DoorEventType::DOOR_CLOSING:
            message.payload.door_event.event = reflow_DoorEvent_EventType_CLOSING;
            break;
        case DoorEventType::DOOR_CLOSED:
            message.payload.door_event.event = reflow_DoorEvent_EventType_CLOSED;
            break;
        case DoorEventType::DOOR_ENABLED:
            message.payload.door_event.event = reflow_DoorEvent_EventType_ENABLED;
            break;
        case DoorEventType::DOOR_DISABLED:
            message.payload.door_event.event = reflow_DoorEvent_EventType_DISABLED;
            break;
        case DoorEventType::DOOR_POSITION:
            message.payload.door_event.event = reflow_DoorEvent_EventType_POSITION_CHANGED;
            break;
    }
    
    // Serialize the message
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    bool status = pb_encode(&stream, reflow_HardwareMessage_fields, &message);
    
    if (status) {
        // Publish via the message event adapter
        MessageEventAdapter::getInstance().postMessage(buffer, stream.bytes_written);
    }
    
    // Also emit legacy event for backward compatibility
    emitEvent(type, positionPercent);
} 
#include "services/buzzer_service.h"
#include "services/communication_service.h"
#include "constants.h"
#include "hardware/pwm.h"

BuzzerService& BuzzerService::getInstance() {
    static BuzzerService instance;
    return instance;
}

void BuzzerService::init() {
    // No hardware initialization needed since we're sending commands to the other Pico
}

void BuzzerService::setEnabled(bool enabled) {
    this->enabled = enabled;
}

bool BuzzerService::isEnabled() const {
    return enabled;
}

void BuzzerService::playHighTone(uint32_t duration_ms) {
    if (!enabled) return;
    playTone(BASE_FREQUENCY + 500, duration_ms);  // 4.5kHz
}

void BuzzerService::playMediumTone(uint32_t duration_ms) {
    if (!enabled) return;
    playTone(BASE_FREQUENCY, duration_ms);  // 4.0kHz
}

void BuzzerService::playLowTone(uint32_t duration_ms) {
    if (!enabled) return;
    playTone(BASE_FREQUENCY - 500, duration_ms);  // 3.5kHz
}

void BuzzerService::playTone(uint32_t frequency, uint32_t duration_ms) {
    // Send a single beep command
    sendBuzzerCommand(0, frequency, duration_ms);
}

void BuzzerService::playPattern(uint32_t pattern, uint32_t frequency, uint32_t duration_ms) {
    // Send a pattern command
    sendBuzzerCommand(pattern, frequency, duration_ms);
}

void BuzzerService::sendBuzzerCommand(uint32_t pattern, uint32_t frequency, uint32_t duration_ms) {
    // Create and send a buzzer command to the other Pico
    reflow_ControllerCommand command = reflow_ControllerCommand_init_zero;
    command.command = reflow_ControllerCommand_CommandType_BUZZER_BEEP;
    command.buzzer_pattern = pattern;
    command.buzzer_frequency = frequency;
    command.buzzer_duration = duration_ms;

    // Send the command through the communication service
    CommunicationService::getInstance().sendCommand(command);
}

void BuzzerService::buzzerTaskWrapper(void* pvParameters) {
    static_cast<BuzzerService*>(pvParameters)->buzzerTask();
}

void BuzzerService::buzzerTask() {
    // Task is kept for future use if needed, but currently just sleeps
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
} 
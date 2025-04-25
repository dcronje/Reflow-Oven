#include "services/buzzer_service.h"
#include "constants.h"
#include "hardware/pwm.h"

BuzzerService& BuzzerService::getInstance() {
    static BuzzerService instance;
    return instance;
}

BuzzerService::BuzzerService() : buzzerPin(BUZZER_PIN) {
    commandQueue = xQueueCreate(10, sizeof(BuzzerCommand));
}

void BuzzerService::init() {
    gpio_set_function(buzzerPin, GPIO_FUNC_PWM);
    gpio_set_dir(buzzerPin, GPIO_OUT);
    gpio_put(buzzerPin, 0);

    xTaskCreate(buzzerTaskWrapper, "BuzzerTask", 256, this, 1, &taskHandle);
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
    BuzzerCommand cmd = {frequency, duration_ms};
    xQueueSend(commandQueue, &cmd, 0);
}

void BuzzerService::buzzerTaskWrapper(void* pvParameters) {
    static_cast<BuzzerService*>(pvParameters)->buzzerTask();
}

void BuzzerService::buzzerTask() {
    BuzzerCommand cmd;
    while (true) {
        if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            // Calculate PWM parameters
            uint32_t slice_num = pwm_gpio_to_slice_num(buzzerPin);
            uint32_t chan = pwm_gpio_to_channel(buzzerPin);
            
            // Set PWM for square wave at specified frequency
            float divider = 125.0f / cmd.frequency;  // 125MHz / frequency
            uint16_t wrap = 65535;
            
            pwm_set_clkdiv(slice_num, divider);
            pwm_set_wrap(slice_num, wrap);
            pwm_set_chan_level(slice_num, chan, wrap / 2);  // 50% duty cycle for square wave
            
            // Enable PWM
            pwm_set_enabled(slice_num, true);
            
            // Wait for duration
            vTaskDelay(pdMS_TO_TICKS(cmd.duration_ms));
            
            // Disable PWM
            pwm_set_enabled(slice_num, false);
        }
    }
} 
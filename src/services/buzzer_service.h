#pragma once

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

class BuzzerService {
public:
    static BuzzerService& getInstance();

    void init();
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Generic tone functions with frequencies around 4kHz baseline
    void playHighTone(uint32_t duration_ms);    // 4.5kHz
    void playMediumTone(uint32_t duration_ms);  // 4.0kHz
    void playLowTone(uint32_t duration_ms);     // 3.5kHz

private:
    BuzzerService();
    void playTone(uint32_t frequency, uint32_t duration_ms);
    static void buzzerTaskWrapper(void* pvParameters);
    void buzzerTask();

    static constexpr uint32_t BASE_FREQUENCY = 4000;  // 4kHz baseline

    bool enabled = true;
    uint8_t buzzerPin;
    TaskHandle_t taskHandle;
    QueueHandle_t commandQueue;

    struct BuzzerCommand {
        uint32_t frequency;
        uint32_t duration_ms;
    };
}; 
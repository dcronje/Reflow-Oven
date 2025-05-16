#pragma once

#include "core/service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "pico/stdlib.h"
#include <cstdint>

class UIViewService; // Forward declaration

// Interaction types
enum class Interaction {
    NONE,
    ENTER,
    BACK,
    UP,
    DOWN,
};

class InteractionService : public Service {
public:
    static InteractionService& getInstance();

    // Initialization and control
    void init();
    void handleInteraction(Interaction interaction);

    // ISR handlers
    void gpioISR(uint gpio, uint32_t events);
    static void debounceTimerCallback(TimerHandle_t xTimer);
    static void longPressTimerCallback(TimerHandle_t xTimer);

private:
    InteractionService() = default;
    ~InteractionService() = default;
    InteractionService(const InteractionService&) = delete;
    InteractionService& operator=(const InteractionService&) = delete;

    // Task entry points
    static void interactionTask(void* params);

    // Static member variables
    static QueueHandle_t interactionQueue;
    static TimerHandle_t debounceTimer;
    static TimerHandle_t longPressTimer;
    static volatile bool buttonState;
    static volatile bool longPressHandled;
    static volatile int32_t encoderPosition;
    static UIViewService* uiService;
    TaskHandle_t taskHandle = nullptr;
};


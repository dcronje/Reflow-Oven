#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "pico/stdlib.h"

#include "interaction_types.h"

class UIViewService; // Forward declaration

class InteractionService {
public:
    static InteractionService& getInstance();

    void init(UIViewService* ui);
    void start();
    void gpioISR(uint gpio, uint32_t events);  // Called by sharedISR

private:
    InteractionService() = default;

    static void taskEntry(void* param);
    void handleInteraction(InteractionType type);

    void handleEncoderISR(uint gpio, uint32_t events);
    void handleButtonISR(uint gpio, uint32_t events);

    static void debounceTimerCallback(TimerHandle_t xTimer);
    static void longPressTimerCallback(TimerHandle_t xTimer);

    TaskHandle_t taskHandle = nullptr;
    static QueueHandle_t interactionQueue;

    static TimerHandle_t debounceTimer;
    static TimerHandle_t longPressTimer;

    static volatile bool buttonState;
    static volatile bool longPressHandled;
    static volatile int32_t encoderPosition;

    static UIViewService* uiService;
};


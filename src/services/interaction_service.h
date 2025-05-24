#pragma once

#include <pb.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "core/service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include <cstdint>
#include "library/protos/common.pb.h"
#include "library/protos/controls.pb.h"

class UIViewService; // Forward declaration

// Interaction types - mapped to all possible input events
enum class Interaction {
    NONE,
    // Encoder interactions
    ENCODER_UP,
    ENCODER_DOWN,
    ENCODER_PRESS,
    ENCODER_LONG_PRESS,
    
    // Button 1 interactions
    BUTTON_1_PRESS,
    BUTTON_1_LONG_PRESS,
    
    // Button 2 interactions
    BUTTON_2_PRESS,
    BUTTON_2_LONG_PRESS,
    
    // Button 3 interactions
    BUTTON_3_PRESS,
    BUTTON_3_LONG_PRESS,
    
    // Button 4 interactions
    BUTTON_4_PRESS,
    BUTTON_4_LONG_PRESS
};

class InteractionService : public Service {
public:
    static InteractionService& getInstance();

    // Initialization and control
    void init() override;
    void handleInteraction(Interaction interaction);

    // Input event handler for communication service
    static void handleInputEvent(reflow_InputEvent_InputType type, int32_t encoder_steps);

private:
    InteractionService() = default;
    ~InteractionService() = default;
    InteractionService(const InteractionService&) = delete;
    InteractionService& operator=(const InteractionService&) = delete;

    // Task entry points
    static void interactionTask(void* params);

    // Static member variables
    static QueueHandle_t interactionQueue;
    static UIViewService* uiService;
    TaskHandle_t taskHandle = nullptr;
};


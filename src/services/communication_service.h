#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "constants.h"
#include "library/protos/common.pb.h"
#include "library/protos/controls.pb.h"
#include <cstring>
#include <cctype>

// Maximum size of a protobuf message
#define MAX_MESSAGE_SIZE 256

// Message data structure for queue
struct MessageData {
    uint8_t buffer[MAX_MESSAGE_SIZE];
    size_t length;
};

// Input event callback type
using InputEventCallback = void (*)(reflow_InputEvent_InputType type, int32_t encoder_steps);

class CommunicationService {
public:
    static CommunicationService& getInstance();
    void init();
    
    // Command sending methods
    void sendCommand(const reflow_ControllerCommand& command);
    void sendBuzzerCommand(uint32_t frequency, uint32_t duration, uint32_t pattern = 0);
    void sendLedCommand(uint8_t r, uint8_t g, uint8_t b);
    void sendMessage(const MessageData& msgData);  // New method for sending raw messages
    
    // Input event handling
    void registerInputEventHandler(InputEventCallback callback);
    void unregisterInputEventHandler();

private:
    CommunicationService() = default;
    ~CommunicationService() = default;
    CommunicationService(const CommunicationService&) = delete;
    CommunicationService& operator=(const CommunicationService&) = delete;

    static void communicationTask(void* params);
    void uartTask();
    void processReceivedMessage(const uint8_t* data, size_t length);
    void handleInputEvent(const reflow_InputEvent& event);

    // Static members for FreeRTOS queues
    static QueueHandle_t txQueue;  // Queue for outgoing messages
    static QueueHandle_t rxQueue;  // Queue for incoming complete messages
    TaskHandle_t taskHandle = nullptr;
    uint32_t sequenceId = 0;
    
    // Input event handling
    InputEventCallback inputEventCallback = nullptr;
}; 
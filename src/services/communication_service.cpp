#include "services/communication_service.h"
#include "services/buzzer_service.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/regs/uart.h"  // Add this for UART register definitions
#include "constants.h"
#include "isr_handlers.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include <cstring>
#include <cctype>  // for isxdigit

// Static member initialization
QueueHandle_t CommunicationService::txQueue = nullptr;
QueueHandle_t CommunicationService::rxQueue = nullptr;

CommunicationService& CommunicationService::getInstance() {
    static CommunicationService instance;
    return instance;
}

void CommunicationService::init() {
    // Create queues for TX and RX
    txQueue = xQueueCreate(10, sizeof(MessageData));  // Queue for outgoing messages
    rxQueue = xQueueCreate(10, sizeof(MessageData));  // Queue for incoming complete messages

    // First deinit UART1 to ensure clean state
    uart_deinit(UART);
    
    // Basic GPIO setup
    gpio_set_function(UART_TX_GPIO, GPIO_FUNC_UART);  // TX
    gpio_set_function(UART_RX_GPIO, GPIO_FUNC_UART);  // RX
    
    // Simple UART init
    uart_init(UART, UART_BAUD_RATE);
    uart_set_format(UART, 8, 1, UART_PARITY_NONE);
    
    // Write initial test message
    const char* test_msg = "\n=== UART1 Test (Basic Polling Mode) ===\n";
    uart_write_blocking(UART, (const uint8_t*)test_msg, strlen(test_msg));
    
    // Create communication task
    xTaskCreate(communicationTask, "CommTask", 1024, this, 1, &taskHandle);
}

void CommunicationService::registerInputEventHandler(InputEventCallback callback) {
    inputEventCallback = callback;
}

void CommunicationService::unregisterInputEventHandler() {
    inputEventCallback = nullptr;
}

void CommunicationService::sendBuzzerCommand(uint32_t frequency, uint32_t duration, uint32_t pattern) {
    reflow_ControlsMessage message = reflow_ControlsMessage_init_zero;
    message.has_base = true;
    message.base.type = reflow_BaseMessage_MessageType_COMMAND;
    message.base.sequence_id = ++sequenceId;
    message.base.timestamp = to_ms_since_boot(get_absolute_time());
    message.which_payload = reflow_ControlsMessage_command_tag;
    message.payload.command.command = reflow_ControllerCommand_CommandType_BUZZER_BEEP;
    message.payload.command.buzzer_frequency = frequency;
    message.payload.command.buzzer_duration = duration;
    message.payload.command.buzzer_pattern = pattern;

    // Encode message
    MessageData msgData;
    pb_ostream_t stream = pb_ostream_from_buffer(msgData.buffer, sizeof(msgData.buffer));
    
    if (pb_encode(&stream, reflow_ControlsMessage_fields, &message)) {
        msgData.length = stream.bytes_written;
        sendMessage(msgData);
    }
}

void CommunicationService::sendLedCommand(uint8_t r, uint8_t g, uint8_t b) {
    reflow_ControlsMessage message = reflow_ControlsMessage_init_zero;
    message.has_base = true;
    message.base.type = reflow_BaseMessage_MessageType_COMMAND;
    message.base.sequence_id = ++sequenceId;
    message.base.timestamp = to_ms_since_boot(get_absolute_time());
    message.which_payload = reflow_ControlsMessage_command_tag;
    message.payload.command.command = reflow_ControllerCommand_CommandType_SET_LED;
    message.payload.command.led_color = (r << 16) | (g << 8) | b;

    // Encode message
    MessageData msgData;
    pb_ostream_t stream = pb_ostream_from_buffer(msgData.buffer, sizeof(msgData.buffer));
    
    if (pb_encode(&stream, reflow_ControlsMessage_fields, &message)) {
        msgData.length = stream.bytes_written;
        sendMessage(msgData);
    }
}

void CommunicationService::sendCommand(const reflow_ControllerCommand& command) {
    reflow_ControlsMessage message = reflow_ControlsMessage_init_zero;
    message.has_base = true;
    message.base.type = reflow_BaseMessage_MessageType_COMMAND;
    message.base.sequence_id = ++sequenceId;
    message.base.timestamp = to_ms_since_boot(get_absolute_time());
    message.which_payload = reflow_ControlsMessage_command_tag;
    message.payload.command = command;

    // Encode message
    MessageData msgData;
    pb_ostream_t stream = pb_ostream_from_buffer(msgData.buffer, sizeof(msgData.buffer));
    
    if (pb_encode(&stream, reflow_ControlsMessage_fields, &message)) {
        msgData.length = stream.bytes_written;
        sendMessage(msgData);
    }
}

void CommunicationService::sendMessage(const MessageData& msgData) {
    xQueueSend(txQueue, &msgData, 0);
}

/**
 * Communication Service Implementation
 * 
 * This service handles bidirectional communication between the master and control Picos
 * using UART. Messages are encoded using Protocol Buffers (nanopb) and transmitted
 * as hex-encoded strings for reliability.
 * 
 * Message Flow:
 * 1. Outgoing messages (TX):
 *    - Messages are created and encoded using nanopb
 *    - Encoded binary data is converted to hex strings
 *    - Hex strings are sent over UART with newline termination
 * 
 * 2. Incoming messages (RX):
 *    - Hex-encoded strings are read from UART
 *    - Strings are converted back to binary data
 *    - Complete messages are queued for processing
 *    - Messages are decoded and handled based on type
 */

void CommunicationService::uartTask() {
    // Buffer for collecting incoming message bytes
    uint8_t rxBuffer[MAX_MESSAGE_SIZE];
    size_t rxIndex = 0;  // Current position in rxBuffer
    
    // Buffer for collecting hex digits (2 hex chars = 1 byte)
    char hexBuffer[3] = {0};
    size_t hexIndex = 0;
    bool collectingHex = false;  // Whether we're currently collecting a hex byte
    
    // Message tracking for debugging
    uint32_t messagesReceived = 0;   // Total messages received
    uint32_t messagesProcessed = 0;  // Total messages processed
    
    // Timing control for message processing
    uint32_t lastProcessTime = 0;                    // Last time we processed a message
    const uint32_t PROCESS_INTERVAL = 2;             // Minimum ms between processing messages
    MessageData msgData;                             // Reusable message buffer

    while (true) {
        // Get current time for timing control
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        
        // Step 1: Handle outgoing messages (TX)
        // Check if there are any messages to send
        if (xQueueReceive(txQueue, &msgData, 0) == pdPASS) {
            // Convert binary message to hex and send
            // Each byte becomes two hex characters
            for(size_t i = 0; i < msgData.length; i++) {
                char hex[3];
                snprintf(hex, sizeof(hex), "%02x", msgData.buffer[i]);
                uart_write_blocking(UART, (const uint8_t*)hex, 2);
            }
            // Terminate message with newline
            uart_write_blocking(UART, (const uint8_t*)"\n", 1);
        }

        // Step 2: Process any complete messages from RX queue
        // This is done before reading new data to prevent queue buildup
        // Only process if enough time has passed since last processing
        if (currentTime - lastProcessTime >= PROCESS_INTERVAL) {
            if (xQueueReceive(rxQueue, &msgData, 0) == pdPASS) {
                messagesProcessed++;
                printf("Processing message %lu/%lu\n", messagesProcessed, messagesReceived);
                processReceivedMessage(msgData.buffer, msgData.length);
                lastProcessTime = currentTime;
            }
        }

        // Step 3: Read and process incoming data (RX)
        if (uart_is_readable(UART)) {
            uint8_t data;
            uart_read_blocking(UART, &data, 1);
            
            // Process hex digits
            if (isxdigit(data)) {
                // Start collecting a new hex byte if we weren't already
                if (!collectingHex) {
                    collectingHex = true;
                    hexIndex = 0;
                    rxIndex = 0;
                }
                
                // Add hex digit to buffer
                hexBuffer[hexIndex++] = data;
                
                // When we have two hex digits, convert to a byte
                if (hexIndex == 2) {
                    hexBuffer[hexIndex] = '\0';
                    uint8_t byte;
                    sscanf(hexBuffer, "%hhx", &byte);
                    
                    // Add byte to message buffer if there's space
                    if (rxIndex < MAX_MESSAGE_SIZE) {
                        rxBuffer[rxIndex++] = byte;
                    } else {
                        // Message too large - reset collection
                        printf("WARNING: Message too large (%d bytes), truncating\n", (int)rxIndex);
                        collectingHex = false;
                        hexIndex = 0;
                        rxIndex = 0;
                    }
                    hexIndex = 0;
                }
            } 
            // Newline indicates end of message
            else if (data == '\n') {
                // If we have collected any data, queue it as a complete message
                if (rxIndex > 0) {
                    MessageData rxMsg;
                    memcpy(rxMsg.buffer, rxBuffer, rxIndex);
                    rxMsg.length = rxIndex;
                    
                    // Try to queue the message with a small timeout
                    if (xQueueSend(rxQueue, &rxMsg, pdMS_TO_TICKS(1)) != pdPASS) {
                        printf("WARNING: RX queue full, message dropped\n");
                        
                        // If queue is full, process one message immediately to make space
                        if (xQueueReceive(rxQueue, &msgData, 0) == pdPASS) {
                            messagesProcessed++;
                            printf("Processing message %lu/%lu (forced)\n", messagesProcessed, messagesReceived);
                            processReceivedMessage(msgData.buffer, msgData.length);
                            lastProcessTime = currentTime;
                            
                            // Now try to queue the new message again
                            if (xQueueSend(rxQueue, &rxMsg, 0) == pdPASS) {
                                messagesReceived++;
                                printf("Message queued after forced processing (%d bytes)\n", (int)rxIndex);
                            }
                        }
                    } else {
                        messagesReceived++;
                        printf("Message queued (%d bytes), total received: %lu\n", 
                               (int)rxIndex, messagesReceived);
                    }
                }
                // Reset collection for next message
                collectingHex = false;
                hexIndex = 0;
                rxIndex = 0;
            }
        }
        
        // Small delay to prevent tight polling while maintaining responsiveness
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
 * Process a received message
 * 
 * This function decodes the binary message using nanopb and handles it based on type.
 * Currently supports:
 * - Input events (encoder turns, button presses)
 * - Other message types are logged but not handled
 */
void CommunicationService::processReceivedMessage(const uint8_t* data, size_t length) {
    reflow_ControlsMessage message = reflow_ControlsMessage_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(data, length);
    
    // Attempt to decode the message
    if (pb_decode(&stream, reflow_ControlsMessage_fields, &message)) {
        // Handle different message types
        if (message.which_payload == reflow_ControlsMessage_input_event_tag) {
            printf("Decoded input event - Type: %d, Steps: %d\n", 
                   message.payload.input_event.type, 
                   message.payload.input_event.encoder_steps);
            handleInputEvent(message.payload.input_event);
        } else {
            printf("Received non-input event message type: %d\n", message.which_payload);
        }
    } else {
        // Log decoding errors with raw message data for debugging
        printf("ERROR: Failed to decode message (%d bytes)\n", (int)length);
        printf("Raw hex: ");
        for(size_t i = 0; i < length; i++) {
            printf("%02x ", data[i]);
        }
        printf("\n");
    }
}

void CommunicationService::handleInputEvent(const reflow_InputEvent& event) {
    if (inputEventCallback) {
        printf("Input event: %d, %d\n", event.type, event.encoder_steps);
        inputEventCallback(event.type, event.encoder_steps);
    }
}

void CommunicationService::communicationTask(void* params) {
    auto* instance = static_cast<CommunicationService*>(params);
    instance->uartTask();
} 
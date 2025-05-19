#include "core/message_event_bus.h"
#include <algorithm>

MessageEventBus& MessageEventBus::getInstance() {
    static MessageEventBus instance;
    return instance;
}

MessageEventBus::MessageEventBus() {
    // Initialize with reasonable capacity
    globalHandlers.reserve(10);
    typeHandlers.reserve(20);
    
    // Create the mutex semaphores
    globalHandlerMutex = xSemaphoreCreateMutex();
    typeHandlerMutex = xSemaphoreCreateMutex();
}

MessageEventBus::~MessageEventBus() {
    if (globalHandlerMutex != nullptr) {
        vSemaphoreDelete(globalHandlerMutex);
    }
    if (typeHandlerMutex != nullptr) {
        vSemaphoreDelete(typeHandlerMutex);
    }
}

void MessageEventBus::registerHandler(MessageHandler* handler) {
    if (handler == nullptr) return;
    
    // Take the mutex
    if (xSemaphoreTake(globalHandlerMutex, portMAX_DELAY) == pdTRUE) {
        // Check if this handler is already registered
        auto it = std::find(globalHandlers.begin(), globalHandlers.end(), handler);
        if (it == globalHandlers.end()) {
            // Handler not found, add it
            globalHandlers.push_back(handler);
        }
        
        // Release the mutex
        xSemaphoreGive(globalHandlerMutex);
    }
}

void MessageEventBus::registerTypeHandler(uint32_t messageType, MessageHandler* handler) {
    if (handler == nullptr) return;
    
    // Take the mutex
    if (xSemaphoreTake(typeHandlerMutex, portMAX_DELAY) == pdTRUE) {
        // Check if this type+handler combination is already registered
        auto it = std::find_if(
            typeHandlers.begin(), 
            typeHandlers.end(),
            [messageType, handler](const auto& pair) {
                return pair.first == messageType && pair.second == handler;
            }
        );
        
        if (it == typeHandlers.end()) {
            // Combination not found, add it
            typeHandlers.emplace_back(messageType, handler);
        }
        
        // Release the mutex
        xSemaphoreGive(typeHandlerMutex);
    }
}

void MessageEventBus::unregisterHandler(MessageHandler* handler) {
    if (handler == nullptr) return;
    
    // Take global handler mutex
    if (xSemaphoreTake(globalHandlerMutex, portMAX_DELAY) == pdTRUE) {
        // Remove the handler from global handlers
        globalHandlers.erase(
            std::remove(globalHandlers.begin(), globalHandlers.end(), handler),
            globalHandlers.end()
        );
        
        // Release the mutex
        xSemaphoreGive(globalHandlerMutex);
    }
    
    // Take type handler mutex
    if (xSemaphoreTake(typeHandlerMutex, portMAX_DELAY) == pdTRUE) {
        // Remove the handler from type handlers
        typeHandlers.erase(
            std::remove_if(
                typeHandlers.begin(), 
                typeHandlers.end(),
                [handler](const auto& pair) { return pair.second == handler; }
            ),
            typeHandlers.end()
        );
        
        // Release the mutex
        xSemaphoreGive(typeHandlerMutex);
    }
}

void MessageEventBus::unregisterTypeHandler(uint32_t messageType, MessageHandler* handler) {
    if (handler == nullptr) return;
    
    // Take the mutex
    if (xSemaphoreTake(typeHandlerMutex, portMAX_DELAY) == pdTRUE) {
        // Remove specific type+handler combination
        typeHandlers.erase(
            std::remove_if(
                typeHandlers.begin(), 
                typeHandlers.end(),
                [messageType, handler](const auto& pair) {
                    return pair.first == messageType && pair.second == handler;
                }
            ),
            typeHandlers.end()
        );
        
        // Release the mutex
        xSemaphoreGive(typeHandlerMutex);
    }
}

bool MessageEventBus::processMessage(const void* data, size_t size) {
    if (data == nullptr || size == 0) return false;
    
    uint32_t messageType = getMessageType(data, size);
    
    // Make a copy of the handlers to avoid holding locks during processing
    std::vector<MessageHandler*> currentGlobalHandlers;
    std::vector<MessageHandler*> currentTypeHandlers;
    
    // Copy the global handlers while holding the mutex
    if (xSemaphoreTake(globalHandlerMutex, portMAX_DELAY) == pdTRUE) {
        currentGlobalHandlers = globalHandlers;
        xSemaphoreGive(globalHandlerMutex);
    }
    
    // Copy type handlers while holding the mutex
    if (xSemaphoreTake(typeHandlerMutex, portMAX_DELAY) == pdTRUE) {
        // Find all handlers for this message type
        for (const auto& pair : typeHandlers) {
            if (pair.first == messageType) {
                currentTypeHandlers.push_back(pair.second);
            }
        }
        xSemaphoreGive(typeHandlerMutex);
    }
    
    bool processed = false;
    
    // Process with all global handlers
    for (auto handler : currentGlobalHandlers) {
        if (handler != nullptr && handler != this) { // Avoid recursive calls
            if (handler->processMessage(data, size)) {
                processed = true;
            }
        }
    }
    
    // Process with type-specific handlers
    for (auto handler : currentTypeHandlers) {
        // Skip if this handler is also a global handler (avoids duplicates)
        if (handler != nullptr && handler != this &&
            std::find(currentGlobalHandlers.begin(), currentGlobalHandlers.end(), handler) 
                == currentGlobalHandlers.end()) {
            if (handler->processMessage(data, size)) {
                processed = true;
            }
        }
    }
    
    return processed;
}

bool MessageEventBus::processMessage(const std::string& serialized) {
    if (serialized.empty()) return false;
    return processMessage(serialized.data(), serialized.size());
}

bool MessageEventBus::publishMessage(const void* data, size_t size) {
    return processMessage(data, size);
}

bool MessageEventBus::publishMessage(const std::string& serialized) {
    return processMessage(serialized);
}

QueueHandle_t MessageEventBus::createMessageQueue(UBaseType_t length) {
    return xQueueCreate(length, MAX_MESSAGE_SIZE);
}

uint32_t MessageEventBus::getMessageType(const void* data, size_t size) {
    // Attempt to decode the message type from the raw data
    // This is a simplified implementation that assumes the message type is
    // stored in the first 4 bytes of the message
    
    if (data == nullptr || size < 4) {
        return 0;
    }
    
    // Try to extract the message type from the reflow message
    // This needs to be adapted based on how the message type is encoded
    // in your Protocol Buffer messages
    
    // Example: For a ReflowMessage, we might check which_payload field
    uint32_t messageType = 0;
    
    // First 4 bytes could be the message type
    memcpy(&messageType, data, sizeof(uint32_t));
    
    return messageType;
} 
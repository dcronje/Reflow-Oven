#pragma once

#include "core/message_event_bus.h"
#include <memory>
#include <vector>
#include <string>

/**
 * @brief Message type constants
 * 
 * These constants help identify message types for routing.
 */
enum class MessageType : uint32_t {
    UNKNOWN = 0,
    SYSTEM = 1,
    HARDWARE = 2,
    CONTROLS = 3,
    UI = 4
};

/**
 * @brief Message event adapter for communication
 * 
 * This class provides convenience methods for working with serialized
 * messages in the event system.
 */
class MessageEventAdapter {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return MessageEventAdapter& The singleton instance
     */
    static MessageEventAdapter& getInstance();
    
    /**
     * @brief Post a message to the event bus
     * 
     * @param data Raw message data
     * @param size Size of the message data
     * @return true if message was successfully posted
     */
    bool postMessage(const void* data, size_t size);
    
    /**
     * @brief Post a message to the event bus
     * 
     * @param serialized Serialized message as a string 
     * @return true if message was successfully posted
     */
    bool postMessage(const std::string& serialized);
    
    /**
     * @brief Get a unique message ID
     * 
     * @return uint32_t Next message ID
     */
    uint32_t getNextMessageId();
    
    /**
     * @brief Get the current timestamp in milliseconds 
     * 
     * @return uint64_t Current timestamp in milliseconds
     */
    uint64_t getCurrentTimestamp() const;
    
private:
    MessageEventAdapter();
    ~MessageEventAdapter() = default;
    
    // Current message ID counter
    static uint32_t currentMessageId;
}; 
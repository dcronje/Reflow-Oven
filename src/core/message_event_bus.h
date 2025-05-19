#pragma once

#include "core/message_handler.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "src/generated/protos/messages.pb.h"
#include <vector>
#include <string>
#include <cstring>
#include <memory>

/**
 * @brief Message-based event bus that uses Protocol Buffers
 * 
 * This implementation replaces the generic event bus with one that
 * specifically handles Protocol Buffer messages.
 */
class MessageEventBus : public MessageHandler, public MessagePublisher {
public:
    static MessageEventBus& getInstance();
    
    // Delete copy/move constructors and assignment operators
    MessageEventBus(const MessageEventBus&) = delete;
    MessageEventBus& operator=(const MessageEventBus&) = delete;
    MessageEventBus(MessageEventBus&&) = delete;
    MessageEventBus& operator=(MessageEventBus&&) = delete;
    
    /**
     * @brief Register a message handler to receive all messages
     * 
     * @param handler The message handler to register
     */
    void registerHandler(MessageHandler* handler);
    
    /**
     * @brief Register a message handler to receive messages of a specific type
     * 
     * @param messageType The message type to subscribe to
     * @param handler The message handler to register
     */
    void registerTypeHandler(uint32_t messageType, MessageHandler* handler);
    
    /**
     * @brief Unregister a message handler from all message types
     * 
     * @param handler The message handler to unregister
     */
    void unregisterHandler(MessageHandler* handler);
    
    /**
     * @brief Unregister a message handler from a specific message type
     * 
     * @param messageType The message type to unsubscribe from
     * @param handler The message handler to unregister
     */
    void unregisterTypeHandler(uint32_t messageType, MessageHandler* handler);
    
    /**
     * @brief Process a raw message
     * 
     * Implements the MessageHandler interface.
     * 
     * @param data Pointer to message data
     * @param size Size of message data in bytes
     * @return true if message was processed successfully, false otherwise
     */
    bool processMessage(const void* data, size_t size) override;
    
    /**
     * @brief Process a message from a serialized string
     * 
     * Implements the MessageHandler interface.
     * 
     * @param serialized Serialized message as a string
     * @return true if message was processed successfully, false otherwise
     */
    bool processMessage(const std::string& serialized) override;
    
    /**
     * @brief Publish a message
     * 
     * Implements the MessagePublisher interface.
     * 
     * @param data Pointer to message data
     * @param size Size of message data in bytes
     * @return true if message was published successfully, false otherwise
     */
    bool publishMessage(const void* data, size_t size) override;
    
    /**
     * @brief Publish a message from a serialized string
     * 
     * Implements the MessagePublisher interface.
     * 
     * @param serialized Serialized message as a string
     * @return true if message was published successfully, false otherwise
     */
    bool publishMessage(const std::string& serialized) override;
    
    /**
     * @brief Create a queue for receiving messages
     * 
     * @param length The length of the queue
     * @return QueueHandle_t The created queue
     */
    static QueueHandle_t createMessageQueue(UBaseType_t length = 10);
    
    /**
     * @brief Maximum size of a message
     * 
     * This is used for queue sizing.
     */
    static constexpr size_t MAX_MESSAGE_SIZE = 128; // Adjust based on expected message sizes
    
private:
    MessageEventBus();
    ~MessageEventBus();
    
    // Extract the message type from a raw message
    uint32_t getMessageType(const void* data, size_t size);
    
    // Global handlers receive all messages
    std::vector<MessageHandler*> globalHandlers;
    
    // Type handlers receive only messages of specific types
    std::vector<std::pair<uint32_t, MessageHandler*>> typeHandlers;
    
    // Mutexes for thread safety
    SemaphoreHandle_t globalHandlerMutex;
    SemaphoreHandle_t typeHandlerMutex;
}; 
#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>

// Forward declarations
class MessageHandler;
class MessagePublisher;

/**
 * @brief Base interface for message handling
 * 
 * This abstract class provides a framework for message handlers that can be used
 * across different projects. It defines the core interface for message processing
 * without any project-specific implementation details.
 */
class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    
    /**
     * @brief Process a raw message
     * 
     * @param data Pointer to message data
     * @param size Size of message data in bytes
     * @return true if message was processed successfully, false otherwise
     */
    virtual bool processMessage(const void* data, size_t size) = 0;
    
    /**
     * @brief Process a message from a serialized string
     * 
     * @param serialized Serialized message as a string
     * @return true if message was processed successfully, false otherwise
     */
    virtual bool processMessage(const std::string& serialized) = 0;
};

/**
 * @brief Base interface for message publishing
 * 
 * This abstract class provides a framework for message publishers that can be used
 * across different projects. It defines the core interface for sending messages
 * without any project-specific implementation details.
 */
class MessagePublisher {
public:
    virtual ~MessagePublisher() = default;
    
    /**
     * @brief Publish a serialized message
     * 
     * @param data Pointer to message data
     * @param size Size of message data in bytes
     * @return true if message was published successfully, false otherwise
     */
    virtual bool publishMessage(const void* data, size_t size) = 0;
    
    /**
     * @brief Publish a message from a serialized string
     * 
     * @param serialized Serialized message as a string
     * @return true if message was published successfully, false otherwise
     */
    virtual bool publishMessage(const std::string& serialized) = 0;
}; 
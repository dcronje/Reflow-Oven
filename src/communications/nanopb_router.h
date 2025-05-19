#pragma once

#include "core/message_handler.h"
#include <functional>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

// Forward declarations for nanopb types
struct ReflowMessage;
struct SystemMessage;
struct HardwareMessage;
struct ControlsMessage;
struct UIMessage;

/**
 * @brief Message router for nanopb messages
 * 
 * This class handles translation between nanopb protocol buffer messages
 * and internal message formats, routing messages to appropriate handlers.
 */
class NanopbRouter : public MessageHandler, public MessagePublisher {
public:
    // Singleton access
    static NanopbRouter& getInstance();

    // Delete copy/move constructors and assignment operators
    NanopbRouter(const NanopbRouter&) = delete;
    NanopbRouter& operator=(const NanopbRouter&) = delete;
    NanopbRouter(NanopbRouter&&) = delete;
    NanopbRouter& operator=(NanopbRouter&&) = delete;

    /**
     * @brief Initialize the router
     * 
     * @param targetHandler The message handler to receive decoded messages
     * @param targetPublisher The message publisher to receive encoded messages
     * @return true if initialization was successful, false otherwise
     */
    bool init(MessageHandler* targetHandler, MessagePublisher* targetPublisher);

    /**
     * @brief Process incoming raw nanopb messages
     * 
     * Implements the MessageHandler interface.
     * Decodes the message and routes it to the target handler.
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
     * Encodes the message and routes it to the target publisher.
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
     * @brief Create a ReflowMessage
     * 
     * Factory method for creating a ReflowMessage (top-level message type)
     * 
     * @return A new ReflowMessage
     */
    std::unique_ptr<ReflowMessage> createReflowMessage();

    /**
     * @brief Create a SystemMessage
     * 
     * Factory method for creating a SystemMessage
     * 
     * @return A new SystemMessage
     */
    std::unique_ptr<SystemMessage> createSystemMessage();

    /**
     * @brief Create a HardwareMessage
     * 
     * Factory method for creating a HardwareMessage
     * 
     * @return A new HardwareMessage
     */
    std::unique_ptr<HardwareMessage> createHardwareMessage();

    /**
     * @brief Create a ControlsMessage
     * 
     * Factory method for creating a ControlsMessage
     * 
     * @return A new ControlsMessage
     */
    std::unique_ptr<ControlsMessage> createControlsMessage();

    /**
     * @brief Create a UIMessage
     * 
     * Factory method for creating a UIMessage
     * 
     * @return A new UIMessage
     */
    std::unique_ptr<UIMessage> createUIMessage();

private:
    NanopbRouter();
    ~NanopbRouter();

    // Target handler and publisher
    MessageHandler* targetHandler;
    MessagePublisher* targetPublisher;

    // Decode a nanopb message
    bool decodeMessage(const void* data, size_t size, ReflowMessage& message);

    // Encode a nanopb message
    bool encodeMessage(const ReflowMessage& message, std::vector<uint8_t>& output);

    // Flag to track initialization status
    bool initialized = false;
}; 
#pragma once

#include "core/message_handler.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <cstdint> // for uint*_t types

/**
 * @brief UART communication manager for inter-processor communication
 * 
 * This class handles UART communication with the secondary controller,
 * encoding and decoding messages using the nanopb protocol.
 */
class UartManager : public MessagePublisher, public MessageHandler {
public:
    // Singleton access
    static UartManager& getInstance();

    // Delete copy/move constructors and assignment operators
    UartManager(const UartManager&) = delete;
    UartManager& operator=(const UartManager&) = delete;
    UartManager(UartManager&&) = delete;
    UartManager& operator=(UartManager&&) = delete;

    /**
     * @brief Initialize the UART communication
     * 
     * @param rxPin UART RX pin number
     * @param txPin UART TX pin number
     * @param baudRate Baud rate for the UART communication
     * @param messageHandler Handler for received messages
     * @return true if initialization was successful, false otherwise
     */
    bool init(uint8_t rxPin, uint8_t txPin, uint32_t baudRate, MessageHandler* messageHandler);

    /**
     * @brief Process incoming messages
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
     * @brief Check if the UART connection is active
     * 
     * @return true if connection is active, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Update the UART connection
     * 
     * Should be called periodically to process transmit queue.
     */
    void update();

    /**
     * @brief Stop the UART connection
     */
    void stop();

private:
    UartManager();
    ~UartManager();

    // UART hardware instance
    unsigned int uart_inst;
    
    // UART configuration
    uint8_t rxPin;
    uint8_t txPin;
    uint32_t baudRate;
    
    // Message handler for received messages
    MessageHandler* messageHandler;
    
    // Transmit queue and mutex
    std::queue<std::vector<uint8_t>> txQueue;
    std::mutex txMutex;
    
    // Receive buffer
    std::vector<uint8_t> rxBuffer;
    
    // Thread for UART reception
    std::thread rxThread;
    std::atomic<bool> running;
    
    // UART receive thread method
    void receiveTask();
    
    // Protocol framing
    static constexpr uint8_t START_BYTE = 0xAA;
    static constexpr uint8_t END_BYTE = 0x55;
    static constexpr uint8_t ESCAPE_BYTE = 0xCC;
    
    // Encode a message with framing
    std::vector<uint8_t> encodeMessage(const void* data, size_t size);
    
    // Decode a framed message
    bool decodeMessage(const std::vector<uint8_t>& data, std::vector<uint8_t>& result);
    
    // Flag to track initialization status
    bool initialized = false;
}; 
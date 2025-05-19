#pragma once

#include "core/message_handler.h"
#include "core/state_machine_interface.h"
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <vector>

// Forward declarations for nanopb types
struct ReflowMessage;
struct SystemMessage;
struct HardwareMessage;
struct ControlsMessage;
struct UIMessage;

/**
 * @brief System orchestrator for the reflow oven
 * 
 * This class coordinates all subsystems of the reflow oven, handling
 * message routing, state transitions, and overall system control.
 */
class SystemOrchestrator : public MessageHandler {
public:
    // Singleton access
    static SystemOrchestrator& getInstance();

    // Delete copy/move constructors and assignment operators
    SystemOrchestrator(const SystemOrchestrator&) = delete;
    SystemOrchestrator& operator=(const SystemOrchestrator&) = delete;
    SystemOrchestrator(SystemOrchestrator&&) = delete;
    SystemOrchestrator& operator=(SystemOrchestrator&&) = delete;

    /**
     * @brief Initialize the orchestrator
     * 
     * Sets up all subsystems, state machine, and message handlers.
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init();

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
     * @brief Update the system state
     * 
     * Should be called periodically to process queued messages and update the state machine.
     */
    void update();

    /**
     * @brief Get the current system state
     * 
     * @return The current system state name
     */
    std::string getCurrentStateName() const;

    /**
     * @brief Transition to a new system state
     * 
     * @param newState The name of the state to transition to
     * @return true if transition was successful, false otherwise
     */
    bool transitionToState(const std::string& newState);

private:
    SystemOrchestrator();
    ~SystemOrchestrator();

    // Message processing methods
    bool processSystemMessage(const SystemMessage& message);
    bool processHardwareMessage(const HardwareMessage& message);
    bool processControlsMessage(const ControlsMessage& message);
    bool processUIMessage(const UIMessage& message);

    // Dispatch a message to services
    bool dispatchMessage(const ReflowMessage& message);

    // Handle system commands
    bool handleSystemCommand(const SystemMessage& message);
    
    // State machine implementation
    std::unique_ptr<StateMachine> stateMachine;
    
    // Message queue and processing
    std::queue<std::vector<uint8_t>> messageQueue;
    std::mutex queueMutex;
    
    // Safety check before state transitions
    bool isSafeToTransition(const std::string& fromState, const std::string& toState) const;
    
    // Flag to track initialization status
    bool initialized = false;
}; 
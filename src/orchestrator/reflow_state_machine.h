#pragma once

#include "core/state_machine_interface.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

// Forward declare concrete state class
class ReflowState;

/**
 * @brief Concrete implementation of a state machine for the reflow oven
 * 
 * This class implements the StateMachine interface for the specific needs
 * of the reflow oven system.
 */
class ReflowStateMachine : public StateMachine {
public:
    ReflowStateMachine();
    ~ReflowStateMachine() override;
    
    /**
     * @brief Add a state to the state machine
     * 
     * @param state The state to add
     * @return true if state was added successfully, false otherwise
     */
    bool addState(State* state) override;
    
    /**
     * @brief Transition to a different state
     * 
     * @param stateName The name of the state to transition to
     * @return true if transition was successful, false otherwise
     */
    bool transitionTo(const char* stateName) override;
    
    /**
     * @brief Get the current state
     * 
     * @return The current state or nullptr if no state is active
     */
    State* getCurrentState() const override;
    
    /**
     * @brief Update the current state
     * 
     * This calls the update method of the current state.
     */
    void update() override;
    
    /**
     * @brief Check if a transition between states is valid
     * 
     * @param fromState The name of the current state
     * @param toState The name of the target state
     * @return true if the transition is valid, false otherwise
     */
    bool isValidTransition(const char* fromState, const char* toState) const;
    
    /**
     * @brief Initialize the state machine with default states
     * 
     * Sets up all the states needed for the reflow oven and defines valid transitions.
     * 
     * @return true if initialization was successful, false otherwise
     */
    bool init();
    
private:
    // Map of states by name
    std::unordered_map<std::string, ReflowState*> states;
    
    // Current active state
    ReflowState* currentState;
    
    // Valid state transitions (from -> to)
    std::unordered_map<std::string, std::vector<std::string>> validTransitions;
    
    // Initial state name
    static constexpr const char* INITIAL_STATE = "IDLE";
    
    // State ownership
    std::vector<std::unique_ptr<ReflowState>> ownedStates;
}; 
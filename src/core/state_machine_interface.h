#pragma once

// Forward declarations
class State;
class StateMachine;

/**
 * @brief Basic state interface
 */
class State {
public:
    virtual ~State() = default;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update() = 0;
    virtual const char* getName() const = 0;
};

/**
 * @brief Basic state transition interface
 */
class StateTransition {
public:
    virtual ~StateTransition() = default;
    virtual bool isValid() const = 0;
    virtual bool execute() = 0;
};

/**
 * @brief Basic state machine interface
 */
class StateMachine {
public:
    virtual ~StateMachine() = default;
    
    /**
     * @brief Add a state to the state machine
     */
    virtual bool addState(State* state) = 0;
    
    /**
     * @brief Transition to a different state
     */
    virtual bool transitionTo(const char* stateName) = 0;
    
    /**
     * @brief Get the current state
     */
    virtual State* getCurrentState() const = 0;
    
    /**
     * @brief Update the current state
     */
    virtual void update() = 0;
}; 
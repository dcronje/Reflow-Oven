#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct State State;
typedef struct StateMachine StateMachine;

// State interface
struct State {
    void (*onEnter)(State* self);
    void (*onExit)(State* self);
    void (*update)(State* self);
    const char* (*getName)(const State* self);
    void* impl; // For user data
};

// State machine interface
struct StateMachine {
    bool (*addState)(StateMachine* self, State* state);
    bool (*transitionTo)(StateMachine* self, const char* stateName);
    State* (*getCurrentState)(const StateMachine* self);
    void (*update)(StateMachine* self);
    void* impl; // For user data
};

#ifdef __cplusplus
}
#endif 
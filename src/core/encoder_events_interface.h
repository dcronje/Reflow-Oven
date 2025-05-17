#pragma once

// Interface for handling encoder events
class EncoderEventsInterface {
public:
    // Event types enum for scheduling
    enum class EventType {
        UP,
        DOWN,
        PRESS,
        LONG_PRESS
    };

    virtual ~EncoderEventsInterface() = default;
    
    // Core encoder event handlers
    virtual void handleEncoderUp() = 0;
    virtual void handleEncoderDown() = 0;
    virtual void handleEncoderPress() = 0;
    virtual void handleEncoderLongPress() = 0;
}; 
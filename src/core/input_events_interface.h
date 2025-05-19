#pragma once

/**
 * @brief Interface for handling input events from various sources
 * 
 * This interface defines methods for handling events from encoder
 * and button interactions. Any class that wants to receive input
 * events should implement this interface.
 */
class InputEventsInterface {
public:
    virtual ~InputEventsInterface() = default;
    
    // Encoder rotation events
    virtual void handleEncoderUp() {}
    virtual void handleEncoderDown() {}
    
    // Button events (for all buttons including encoder press)
    virtual void handleButtonPress(int buttonId) {}
    virtual void handleButtonLongPress(int buttonId) {}
};

// Button ID constants
enum ButtonId {
    ENCODER_BUTTON = 0,  // The encoder's push button
    BUTTON_1 = 1,        // Top button
    BUTTON_2 = 2,        // Bottom button
    BUTTON_3 = 3,        // Left button
    BUTTON_4 = 4         // Right button
}; 
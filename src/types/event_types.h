#pragma once

#include <string>

// Application-specific event topics
namespace EventTopics {
    inline const std::string SYSTEM = "system";
    inline const std::string DOOR = "door";
    inline const std::string TEMPERATURE = "temperature";
    inline const std::string REFLOW = "reflow";
    inline const std::string PROFILE = "profile";
    inline const std::string UI = "ui";
}

// Application-specific event names
namespace EventNames {
    // System events
    inline const std::string INITIALIZED = "initialized";
    inline const std::string ERROR = "error";
    
    // State events
    inline const std::string STARTED = "started";
    inline const std::string STOPPED = "stopped";
    inline const std::string PAUSED = "paused";
    inline const std::string RESUMED = "resumed";
    inline const std::string COMPLETED = "completed";
    inline const std::string UPDATED = "updated";
    inline const std::string CHANGED = "changed";
    
    // Door events
    inline const std::string OPENED = "opened";
    inline const std::string CLOSED = "closed";
    inline const std::string OPENING = "opening";
    inline const std::string CLOSING = "closing";
    
    // Temperature events
    inline const std::string REACHED_TARGET = "reached_target";
    inline const std::string EXCEEDED_LIMIT = "exceeded_limit";
} 
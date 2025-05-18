#pragma once

#include <string>
#include <cstring>

// Generic Event structure with topic-based messaging
struct Event {
    // Main topic (e.g., "door", "temperature", "reflow")
    std::string topic;
    
    // Specific event within the topic (e.g., "opened", "closed", "updated")
    std::string event;
    
    // Optional payload data
    void* payload = nullptr;
    
    // Size of payload for memory management
    size_t payloadSize = 0;
    
    // Default constructor
    Event() {}
    
    // Constructor for events without payload
    Event(const std::string& t, const std::string& e)
        : topic(t), event(e) {}
        
    // Constructor for events with payload
    Event(const std::string& t, const std::string& e, void* data, size_t size = 0)
        : topic(t), event(e), payload(data), payloadSize(size) {}
        
    // Copy constructor (deep copy for payload)
    Event(const Event& other)
        : topic(other.topic), event(other.event), payloadSize(other.payloadSize) {
        if (other.payload && other.payloadSize > 0) {
            payload = malloc(other.payloadSize);
            if (payload) {
                memcpy(payload, other.payload, other.payloadSize);
            }
        }
    }
    
    // Move constructor
    Event(Event&& other) noexcept
        : topic(std::move(other.topic)), event(std::move(other.event)),
          payload(other.payload), payloadSize(other.payloadSize) {
        other.payload = nullptr;
        other.payloadSize = 0;
    }
    
    // Assignment operator (deep copy)
    Event& operator=(const Event& other) {
        if (this != &other) {
            // Free existing payload
            if (payload) {
                free(payload);
                payload = nullptr;
            }
            
            topic = other.topic;
            event = other.event;
            payloadSize = other.payloadSize;
            
            // Copy payload if needed
            if (other.payload && other.payloadSize > 0) {
                payload = malloc(other.payloadSize);
                if (payload) {
                    memcpy(payload, other.payload, other.payloadSize);
                }
            }
        }
        return *this;
    }
    
    // Move assignment
    Event& operator=(Event&& other) noexcept {
        if (this != &other) {
            // Free existing payload
            if (payload) {
                free(payload);
            }
            
            topic = std::move(other.topic);
            event = std::move(other.event);
            payload = other.payload;
            payloadSize = other.payloadSize;
            
            other.payload = nullptr;
            other.payloadSize = 0;
        }
        return *this;
    }
    
    // Destructor to clean up payload
    ~Event() {
        if (payload) {
            free(payload);
            payload = nullptr;
        }
    }
    
    // Template method to safely get payload as a specific type
    template<typename T>
    T* getPayload() const {
        return static_cast<T*>(payload);
    }
}; 
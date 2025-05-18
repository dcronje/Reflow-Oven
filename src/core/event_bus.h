#pragma once

#include "core/event.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include <vector>
#include <string>
#include <cstring>

// EventBus: Central message bus for the application
class EventBus {
public:
    static EventBus& getInstance();
    
    // Register a queue to receive all events
    void registerListener(QueueHandle_t queue);
    
    // Register a queue to receive events for a specific topic
    void registerTopicListener(const std::string& topic, QueueHandle_t queue);
    
    // Unregister a queue from all events
    void unregisterListener(QueueHandle_t queue);
    
    // Unregister a queue from a specific topic
    void unregisterTopicListener(const std::string& topic, QueueHandle_t queue);
    
    // Post an event to all registered listeners
    void postEvent(const Event& event);
    
    // Maximum size of an event queue item (estimate conservatively)
    static constexpr size_t MAX_EVENT_SIZE = sizeof(Event) + 64; // Add extra for string storage
    
private:
    EventBus();
    ~EventBus(); // Destructor to clean up resources
    
    // Global listeners receive all events
    std::vector<QueueHandle_t> globalListeners;
    
    // Topic listeners receive only events for specific topics
    std::vector<std::pair<std::string, QueueHandle_t>> topicListeners;
    
    // Mutexes for thread safety
    SemaphoreHandle_t globalListenerMutex;
    SemaphoreHandle_t topicListenerMutex;
};

// Helper function to create an event queue with proper sizing
inline QueueHandle_t createEventQueue(UBaseType_t length = 10) {
    return xQueueCreate(length, EventBus::MAX_EVENT_SIZE);
} 
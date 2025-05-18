#pragma once

#include "core/event_bus.h"
#include "types/event_types.h"
#include "FreeRTOS.h"
#include "task.h"

// EventSubscriber: Helper class to make it easier to subscribe to and process events
class EventSubscriber {
public:
    // Create a subscriber that listens to all events
    explicit EventSubscriber(UBaseType_t queueLength = 10)
        : eventQueue(createEventQueue(queueLength)) {
        if (eventQueue) {
            EventBus::getInstance().registerListener(eventQueue);
        }
    }
    
    // Create a subscriber that listens to a specific topic
    EventSubscriber(const std::string& topic, UBaseType_t queueLength = 10)
        : eventQueue(createEventQueue(queueLength)),
          subscribedTopic(topic) {
        if (eventQueue && !topic.empty()) {
            EventBus::getInstance().registerTopicListener(topic, eventQueue);
        }
    }
    
    // Clean up resources on destruction
    ~EventSubscriber() {
        if (eventQueue) {
            // Unregister from the event bus (both global and topic-specific)
            EventBus::getInstance().unregisterListener(eventQueue);
            
            if (!subscribedTopic.empty()) {
                EventBus::getInstance().unregisterTopicListener(subscribedTopic, eventQueue);
            }
            
            vQueueDelete(eventQueue);
            eventQueue = nullptr;
        }
    }
    
    // Try to receive an event with timeout (in ticks)
    bool receiveEvent(Event& event, TickType_t timeout = 0) {
        if (!eventQueue) return false;
        return xQueueReceive(eventQueue, &event, timeout) == pdTRUE;
    }
    
    // Try to peek at an event without removing it
    bool peekEvent(Event& event, TickType_t timeout = 0) {
        if (!eventQueue) return false;
        return xQueuePeek(eventQueue, &event, timeout) == pdTRUE;
    }
    
    // Check if queue has events pending
    bool hasEvents() const {
        return eventQueue && uxQueueMessagesWaiting(eventQueue) > 0;
    }
    
    // Clear all events in the queue
    void clearEvents() {
        if (!eventQueue) return;
        
        Event dummy(EventTopics::SYSTEM, EventNames::INITIALIZED);
        while (xQueueReceive(eventQueue, &dummy, 0) == pdTRUE) {
            // Just discard events
        }
    }
    
    // Get the native queue handle for advanced use cases
    QueueHandle_t getQueueHandle() const {
        return eventQueue;
    }
    
    // Subscribe to an additional topic
    void subscribeTopic(const std::string& topic) {
        if (eventQueue && !topic.empty()) {
            EventBus::getInstance().registerTopicListener(topic, eventQueue);
        }
    }
    
    // Unsubscribe from a topic
    void unsubscribeTopic(const std::string& topic) {
        if (eventQueue && !topic.empty()) {
            EventBus::getInstance().unregisterTopicListener(topic, eventQueue);
        }
    }
    
private:
    QueueHandle_t eventQueue = nullptr;
    std::string subscribedTopic;  // Primary topic (if subscribed to one)
    
    // Prevent copying
    EventSubscriber(const EventSubscriber&) = delete;
    EventSubscriber& operator=(const EventSubscriber&) = delete;
}; 
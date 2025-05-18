#include "core/event_bus.h"
#include <algorithm>

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

EventBus::EventBus() {
    // Initialize with reasonable capacity
    globalListeners.reserve(10);
    topicListeners.reserve(20);
    
    // Create the mutex semaphores
    globalListenerMutex = xSemaphoreCreateMutex();
    topicListenerMutex = xSemaphoreCreateMutex();
}

// Destructor to clean up resources (added)
EventBus::~EventBus() {
    if (globalListenerMutex != nullptr) {
        vSemaphoreDelete(globalListenerMutex);
    }
    if (topicListenerMutex != nullptr) {
        vSemaphoreDelete(topicListenerMutex);
    }
}

void EventBus::registerListener(QueueHandle_t queue) {
    if (queue == nullptr) return;
    
    // Take the mutex
    if (xSemaphoreTake(globalListenerMutex, portMAX_DELAY) == pdTRUE) {
        // Check if this queue is already registered
        auto it = std::find(globalListeners.begin(), globalListeners.end(), queue);
        if (it == globalListeners.end()) {
            // Queue not found, add it
            globalListeners.push_back(queue);
        }
        
        // Release the mutex
        xSemaphoreGive(globalListenerMutex);
    }
}

void EventBus::registerTopicListener(const std::string& topic, QueueHandle_t queue) {
    if (queue == nullptr || topic.empty()) return;
    
    // Take the mutex
    if (xSemaphoreTake(topicListenerMutex, portMAX_DELAY) == pdTRUE) {
        // Check if this topic+queue combination is already registered
        auto it = std::find_if(
            topicListeners.begin(), 
            topicListeners.end(),
            [&topic, queue](const auto& pair) {
                return pair.first == topic && pair.second == queue;
            }
        );
        
        if (it == topicListeners.end()) {
            // Combination not found, add it
            topicListeners.emplace_back(topic, queue);
        }
        
        // Release the mutex
        xSemaphoreGive(topicListenerMutex);
    }
}

void EventBus::unregisterListener(QueueHandle_t queue) {
    if (queue == nullptr) return;
    
    // Take global listener mutex
    if (xSemaphoreTake(globalListenerMutex, portMAX_DELAY) == pdTRUE) {
        // Remove the queue from global listeners
        globalListeners.erase(
            std::remove(globalListeners.begin(), globalListeners.end(), queue),
            globalListeners.end()
        );
        
        // Release the mutex
        xSemaphoreGive(globalListenerMutex);
    }
    
    // Take topic listener mutex
    if (xSemaphoreTake(topicListenerMutex, portMAX_DELAY) == pdTRUE) {
        // Remove the queue from topic listeners
        topicListeners.erase(
            std::remove_if(
                topicListeners.begin(), 
                topicListeners.end(),
                [queue](const auto& pair) { return pair.second == queue; }
            ),
            topicListeners.end()
        );
        
        // Release the mutex
        xSemaphoreGive(topicListenerMutex);
    }
}

void EventBus::unregisterTopicListener(const std::string& topic, QueueHandle_t queue) {
    if (queue == nullptr || topic.empty()) return;
    
    // Take the mutex
    if (xSemaphoreTake(topicListenerMutex, portMAX_DELAY) == pdTRUE) {
        // Remove specific topic+queue combination
        topicListeners.erase(
            std::remove_if(
                topicListeners.begin(), 
                topicListeners.end(),
                [&topic, queue](const auto& pair) {
                    return pair.first == topic && pair.second == queue;
                }
            ),
            topicListeners.end()
        );
        
        // Release the mutex
        xSemaphoreGive(topicListenerMutex);
    }
}

void EventBus::postEvent(const Event& event) {
    // Make a copy of the listeners to avoid holding locks during sends
    std::vector<QueueHandle_t> currentGlobalListeners;
    std::vector<QueueHandle_t> currentTopicListeners;
    
    // Copy the global listeners while holding the mutex
    if (xSemaphoreTake(globalListenerMutex, portMAX_DELAY) == pdTRUE) {
        currentGlobalListeners = globalListeners;
        xSemaphoreGive(globalListenerMutex);
    }
    
    // Copy topic listeners while holding the mutex
    if (xSemaphoreTake(topicListenerMutex, portMAX_DELAY) == pdTRUE) {
        // Find all listeners for this topic
        for (const auto& pair : topicListeners) {
            if (pair.first == event.topic) {
                currentTopicListeners.push_back(pair.second);
            }
        }
        xSemaphoreGive(topicListenerMutex);
    }
    
    // Send to all global listeners
    for (auto queue : currentGlobalListeners) {
        if (queue != nullptr) {
            // Use a non-blocking send to avoid deadlocks
            xQueueSend(queue, &event, 0);
        }
    }
    
    // Send to topic-specific listeners
    for (auto queue : currentTopicListeners) {
        // Skip if this queue is also a global listener (avoids duplicates)
        if (queue != nullptr && 
            std::find(currentGlobalListeners.begin(), currentGlobalListeners.end(), queue) 
                == currentGlobalListeners.end()) {
            xQueueSend(queue, &event, 0);
        }
    }
} 
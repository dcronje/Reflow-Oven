#include "core/message_event_adapter.h"
#include "core/message_event_bus.h"
#include <ctime>
#include <cstring>

// Initialize static member
uint32_t MessageEventAdapter::currentMessageId = static_cast<uint32_t>(time(nullptr));

MessageEventAdapter& MessageEventAdapter::getInstance() {
    static MessageEventAdapter instance;
    return instance;
}

MessageEventAdapter::MessageEventAdapter() {
    // Empty constructor
}

uint32_t MessageEventAdapter::getNextMessageId() {
    return ++currentMessageId;
}

uint64_t MessageEventAdapter::getCurrentTimestamp() const {
    return static_cast<uint64_t>(time(nullptr) * 1000); // Current time in ms
}

bool MessageEventAdapter::postMessage(const void* data, size_t size) {
    if (data == nullptr || size == 0) return false;
    
    return MessageEventBus::getInstance().publishMessage(data, size);
}

bool MessageEventAdapter::postMessage(const std::string& serialized) {
    if (serialized.empty()) return false;
    
    return MessageEventBus::getInstance().publishMessage(serialized.data(), serialized.size());
}

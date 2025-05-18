#pragma once

#include <string>
#include <cstring>
#include <cstdint>

namespace EventPayload {

// Base class for all payloads
class PayloadBase {
public:
    virtual ~PayloadBase() = default;
    
    // Create a deep copy of this payload 
    virtual void* clone() const = 0;
    
    // Get size of the payload data
    virtual size_t size() const = 0;
};

// Integer payload wrapper
class IntPayload : public PayloadBase {
public:
    explicit IntPayload(int value) : value(value) {}
    
    void* clone() const override {
        return new IntPayload(value);
    }
    
    size_t size() const override {
        return sizeof(IntPayload);
    }
    
    int getValue() const { return value; }
    
private:
    int value;
};

// Float payload wrapper
class FloatPayload : public PayloadBase {
public:
    explicit FloatPayload(float value) : value(value) {}
    
    void* clone() const override {
        return new FloatPayload(value);
    }
    
    size_t size() const override {
        return sizeof(FloatPayload);
    }
    
    float getValue() const { return value; }
    
private:
    float value;
};

// Boolean payload wrapper
class BoolPayload : public PayloadBase {
public:
    explicit BoolPayload(bool value) : value(value) {}
    
    void* clone() const override {
        return new BoolPayload(value);
    }
    
    size_t size() const override {
        return sizeof(BoolPayload);
    }
    
    bool getValue() const { return value; }
    
private:
    bool value;
};

// String payload wrapper
class StringPayload : public PayloadBase {
public:
    explicit StringPayload(const std::string& value) : value(value) {}
    
    void* clone() const override {
        return new StringPayload(value);
    }
    
    size_t size() const override {
        return sizeof(StringPayload);
    }
    
    const std::string& getValue() const { return value; }
    
private:
    std::string value;
};

// Helper to create common payload types
inline void* createIntPayload(int value) {
    return new IntPayload(value);
}

inline void* createFloatPayload(float value) {
    return new FloatPayload(value);
}

inline void* createBoolPayload(bool value) {
    return new BoolPayload(value);
}

inline void* createStringPayload(const std::string& value) {
    return new StringPayload(value);
}

// Helper to extract values from payload objects
inline int getIntFromPayload(void* payload) {
    if (!payload) return 0;
    auto* p = static_cast<IntPayload*>(payload);
    return p->getValue();
}

inline float getFloatFromPayload(void* payload) {
    if (!payload) return 0.0f;
    auto* p = static_cast<FloatPayload*>(payload);
    return p->getValue();
}

inline bool getBoolFromPayload(void* payload) {
    if (!payload) return false;
    auto* p = static_cast<BoolPayload*>(payload);
    return p->getValue();
}

inline std::string getStringFromPayload(void* payload) {
    if (!payload) return "";
    auto* p = static_cast<StringPayload*>(payload);
    return p->getValue();
}

} // namespace EventPayload 
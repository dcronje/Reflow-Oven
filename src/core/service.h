#pragma once

#include "FreeRTOS.h"
#include "task.h"

class Service {
public:
    virtual ~Service() = default;
    virtual void init() = 0;  // Pure virtual method that all services must implement
}; 
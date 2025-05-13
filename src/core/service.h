#pragma once

#include "FreeRTOS.h"
#include "task.h"

class Service {
public:
    virtual ~Service() = default;
}; 
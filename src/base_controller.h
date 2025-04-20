#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "services/ui_view_service.h"

class BaseController {
public:
    virtual ~BaseController() = default;

    // Setup controller (register views, setup state, etc.)
    virtual void init() {}

    // Hook for view registration
    virtual void registerViews(UIViewService& viewService) = 0;

    // Optional lifecycle control
    virtual void start() {}
    virtual void stop() {}

protected:
    TaskHandle_t taskHandle = nullptr;

    // Create internal task if needed
    void createTask(const char* name, uint16_t stackSize = 1024, UBaseType_t priority = 1);
    static void taskEntry(void* param); // Static entry point wrapper
    virtual void run() {}               // Override to implement behavior
};


#include "base_controller.h"

void BaseController::createTask(const char* name, uint16_t stackSize, UBaseType_t priority) {
    xTaskCreate(taskEntry, name, stackSize, this, priority, &taskHandle);
}

void BaseController::taskEntry(void* param) {
    auto* self = static_cast<BaseController*>(param);
    self->run();
}

#include "isr_handlers.h"
#include "services/interaction_service.h"
#include "constants.h"

void sharedISR(uint gpio, uint32_t events) {
    if (gpio == ENCODER_CLK_GPIO || gpio == ENCODER_DC_GPIO || gpio == ENCODER_SW_GPIO) {
        InteractionService::getInstance().gpioISR(gpio, events);
    }
}

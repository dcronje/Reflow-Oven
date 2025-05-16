#include "services/interaction_service.h"
#include "constants.h"
#include "isr_handlers.h"
#include "services/ui_view_service.h"
#include "services/buzzer_service.h"
#include <cstdio>

QueueHandle_t InteractionService::interactionQueue = nullptr;
TimerHandle_t InteractionService::debounceTimer = nullptr;
TimerHandle_t InteractionService::longPressTimer = nullptr;

volatile bool InteractionService::buttonState = false;
volatile bool InteractionService::longPressHandled = false;
volatile int32_t InteractionService::encoderPosition = 0;

UIViewService* InteractionService::uiService = nullptr;

InteractionService& InteractionService::getInstance() {
    static InteractionService instance;
    return instance;
}

void InteractionService::init() {
    uiService = &UIViewService::getInstance();

    interactionQueue = xQueueCreate(10, sizeof(Interaction));

    // Encoder pin setup
    gpio_init(ENCODER_CLK_GPIO);
    gpio_init(ENCODER_DC_GPIO);
    gpio_set_dir(ENCODER_CLK_GPIO, GPIO_IN);
    gpio_set_dir(ENCODER_DC_GPIO, GPIO_IN);

    gpio_set_irq_enabled(ENCODER_CLK_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true); // ✅ REQUIRED
    gpio_set_irq_enabled(ENCODER_DC_GPIO, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);  // optional, only if needed

    // Button pin setup (encoder switch only)
    debounceTimer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, nullptr, debounceTimerCallback);
    longPressTimer = xTimerCreate("LongPressTimer", pdMS_TO_TICKS(LONG_PRESS_TIME_MS), pdFALSE, nullptr, longPressTimerCallback);
    
    gpio_init(ENCODER_SW_GPIO);
    gpio_set_dir(ENCODER_SW_GPIO, GPIO_IN);
    gpio_pull_up(ENCODER_SW_GPIO); // Assuming active low button
    gpio_set_irq_enabled(ENCODER_SW_GPIO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

    xTaskCreate(interactionTask, "Interaction", 2048, this, 1, &taskHandle);
}

void InteractionService::handleInteraction(Interaction interaction) {
    if (!uiService) return;

    switch (interaction) {
        case Interaction::ENTER:
            printf("ENTER COMMAND\n");
            uiService->handleEncoderPress();
            break;
        case Interaction::BACK:
            printf("BACK COMMAND\n");
            uiService->handleEncoderLongPress();
            break;
        case Interaction::UP:
            printf("UP COMMAND\n");
            uiService->handleEncoderUp();
            break;
        case Interaction::DOWN:
            printf("DOWN COMMAND\n");
            uiService->handleEncoderDown();
            break;
        default: break;
    }
}

void InteractionService::gpioISR(uint gpio, uint32_t events) {
    // Handle encoder
    if (gpio == ENCODER_CLK_GPIO) {
        static int32_t last_level_a = -1;
        static int32_t lastEncoderPosition = 0;
        int level_a = gpio_get(ENCODER_CLK_GPIO);
        int level_b = gpio_get(ENCODER_DC_GPIO);

        if (level_a != last_level_a) {
            last_level_a = level_a;
            if (level_a == 1) {
                encoderPosition += (level_b == 1 ? 1 : -1);
            } else {
                encoderPosition += (level_b == 0 ? 1 : -1);
            }

            Interaction action = Interaction::NONE;
            if (encoderPosition > lastEncoderPosition) {
                action = Interaction::UP;
            } else if (encoderPosition < lastEncoderPosition) {
                action = Interaction::DOWN;
            }
            lastEncoderPosition = encoderPosition;

            if (action != Interaction::NONE) {
                xQueueSendFromISR(interactionQueue, &action, NULL);
            }
        }
    }
    // Handle encoder button
    else if (gpio == ENCODER_SW_GPIO) {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        xTimerResetFromISR(debounceTimer, &higherPriorityTaskWoken);
    }
}

void InteractionService::debounceTimerCallback(TimerHandle_t xTimer) {
    auto& instance = getInstance();
    bool currentLevel = gpio_get(ENCODER_SW_GPIO) == 0; // Assuming active low
    
    if (currentLevel != instance.buttonState) {
        instance.buttonState = currentLevel;
        if (currentLevel) {
            xTimerStart(instance.longPressTimer, 0);
            instance.longPressHandled = false;
        } else {
            xTimerStop(instance.longPressTimer, 0);
            if (!instance.longPressHandled) {
                Interaction action = Interaction::ENTER;
                xQueueSend(instance.interactionQueue, &action, 0);
            }
        }
    }
}

void InteractionService::longPressTimerCallback(TimerHandle_t xTimer) {
    auto& instance = getInstance();
    instance.longPressHandled = true;
    Interaction action = Interaction::BACK;
    xQueueSend(instance.interactionQueue, &action, 0);
}

void InteractionService::interactionTask(void* params) {
    auto* instance = static_cast<InteractionService*>(params);
    Interaction interaction = Interaction::NONE;

    while (true) {
        if (xQueueReceive(instance->interactionQueue, &interaction, portMAX_DELAY) == pdPASS) {
            instance->handleInteraction(interaction);
        }
    }
}

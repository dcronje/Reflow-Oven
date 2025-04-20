#include "interaction_service.h"
#include "constants.h"
#include "isr_handlers.h"
#include "ui_view_service.h"

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

void InteractionService::init(UIViewService* ui) {
    uiService = ui;

    interactionQueue = xQueueCreate(10, sizeof(InteractionType));

    // Encoder pin setup
    gpio_init(ENCODER_CLK_GPIO);
    gpio_init(ENCODER_DC_GPIO);
    gpio_set_dir(ENCODER_CLK_GPIO, GPIO_IN);
    gpio_set_dir(ENCODER_DC_GPIO, GPIO_IN);
    gpio_pull_up(ENCODER_CLK_GPIO);
    gpio_pull_up(ENCODER_DC_GPIO);

    // Button pin setup
    gpio_init(ENCODER_SW_GPIO);
    gpio_set_dir(ENCODER_SW_GPIO, GPIO_IN);
    gpio_pull_up(ENCODER_SW_GPIO);

    gpio_set_irq_enabled(ENCODER_CLK_GPIO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(ENCODER_DC_GPIO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(ENCODER_SW_GPIO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

    debounceTimer = xTimerCreate("debounce", pdMS_TO_TICKS(50), pdFALSE, nullptr, debounceTimerCallback);
    longPressTimer = xTimerCreate("longpress", pdMS_TO_TICKS(500), pdFALSE, nullptr, longPressTimerCallback);

    // Register the shared ISR if not already done
    // Already done in main.cpp
    // gpio_set_irq_callback(sharedISR);
    // irq_set_enabled(IO_IRQ_BANK0, true);
}

void InteractionService::start() {
    xTaskCreate(taskEntry, "InteractionTask", 1024, nullptr, tskIDLE_PRIORITY + 1, &taskHandle);
}

void InteractionService::taskEntry(void* param) {
    InteractionType interaction;
    while (true) {
        if (xQueueReceive(interactionQueue, &interaction, portMAX_DELAY) == pdPASS) {
            InteractionService::getInstance().handleInteraction(interaction);
        }
    }
}

void InteractionService::handleInteraction(InteractionType type) {
    if (!uiService) return;

    switch (type) {
        case InteractionType::UP: uiService->handleEncoderUp(); break;
        case InteractionType::DOWN: uiService->handleEncoderDown(); break;
        case InteractionType::ENTER: uiService->handleEncoderPress(); break;
        case InteractionType::BACK: uiService->handleEncoderLongPress(); break;
        default: break;
    }
}

void InteractionService::gpioISR(uint gpio, uint32_t events) {
    if (gpio == ENCODER_CLK_GPIO || gpio == ENCODER_DC_GPIO) {
        handleEncoderISR(gpio, events);
    } else if (gpio == ENCODER_SW_GPIO) {
        handleButtonISR(gpio, events);
    }
}

void InteractionService::handleEncoderISR(uint gpio, uint32_t events) {
    static int32_t lastPosition = 0;
    int levelA = gpio_get(ENCODER_CLK_GPIO);
    int levelB = gpio_get(ENCODER_DC_GPIO);

    if (levelA == 1) {
        encoderPosition += (levelB == 1 ? 1 : -1);
    } else {
        encoderPosition += (levelB == 0 ? 1 : -1);
    }

    InteractionType action = InteractionType::NONE;
    if (encoderPosition > lastPosition) {
        action = InteractionType::UP;
    } else if (encoderPosition < lastPosition) {
        action = InteractionType::DOWN;
    }
    lastPosition = encoderPosition;

    if (action != InteractionType::NONE) {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(interactionQueue, &action, &higherPriorityTaskWoken);
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
}

void InteractionService::handleButtonISR(uint gpio, uint32_t events) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xTimerResetFromISR(debounceTimer, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void InteractionService::debounceTimerCallback(TimerHandle_t xTimer) {
    bool level = gpio_get(ENCODER_SW_GPIO) == 0; // active low
    if (level != buttonState) {
        buttonState = level;
        if (level) {
            longPressHandled = false;
            xTimerStart(longPressTimer, 0);
        } else {
            xTimerStop(longPressTimer, 0);
            if (!longPressHandled) {
                InteractionType type = InteractionType::ENTER;
                xQueueSend(interactionQueue, &type, 0);
            }
        }
    }
}

void InteractionService::longPressTimerCallback(TimerHandle_t xTimer) {
    longPressHandled = true;
    InteractionType type = InteractionType::BACK;
    xQueueSend(interactionQueue, &type, 0);
}

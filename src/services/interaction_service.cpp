#include "services/interaction_service.h"
#include "constants.h"
#include "isr_handlers.h"
#include "services/ui_view_service.h"
#include "services/buzzer_service.h"
#include "services/communication_service.h"
#include <cstdio>

QueueHandle_t InteractionService::interactionQueue = nullptr;
UIViewService* InteractionService::uiService = nullptr;

InteractionService& InteractionService::getInstance() {
    static InteractionService instance;
    return instance;
}

void InteractionService::init() {
    uiService = &UIViewService::getInstance();
    interactionQueue = xQueueCreate(10, sizeof(Interaction));

    // Register input event handler with communication service
    CommunicationService::getInstance().registerInputEventHandler(handleInputEvent);

    // Create interaction task
    xTaskCreate(interactionTask, "Interaction", 2048, this, 1, &taskHandle);
}

void InteractionService::handleInteraction(Interaction interaction) {
    if (!uiService) return;

    switch (interaction) {
        // Encoder interactions
        case Interaction::ENCODER_UP:
            printf("ENCODER UP\n");
            uiService->handleEncoderUp();
            break;
        case Interaction::ENCODER_DOWN:
            printf("ENCODER DOWN\n");
            uiService->handleEncoderDown();
            break;
        case Interaction::ENCODER_PRESS:
            printf("ENCODER PRESS\n");
            uiService->handleEncoderPress();
            break;
        case Interaction::ENCODER_LONG_PRESS:
            printf("ENCODER LONG PRESS\n");
            uiService->handleEncoderLongPress();
            break;

        // Button 1 interactions
        case Interaction::BUTTON_1_PRESS:
            printf("BUTTON 1 PRESS\n");
            uiService->handleButton1Press();
            break;
        case Interaction::BUTTON_1_LONG_PRESS:
            printf("BUTTON 1 LONG PRESS\n");
            uiService->handleButton1LongPress();
            break;

        // Button 2 interactions
        case Interaction::BUTTON_2_PRESS:
            printf("BUTTON 2 PRESS\n");
            uiService->handleButton2Press();
            break;
        case Interaction::BUTTON_2_LONG_PRESS:
            printf("BUTTON 2 LONG PRESS\n");
            uiService->handleButton2LongPress();
            break;

        // Button 3 interactions
        case Interaction::BUTTON_3_PRESS:
            printf("BUTTON 3 PRESS\n");
            uiService->handleButton3Press();
            break;
        case Interaction::BUTTON_3_LONG_PRESS:
            printf("BUTTON 3 LONG PRESS\n");
            uiService->handleButton3LongPress();
            break;

        // Button 4 interactions
        case Interaction::BUTTON_4_PRESS:
            printf("BUTTON 4 PRESS\n");
            uiService->handleButton4Press();
            break;
        case Interaction::BUTTON_4_LONG_PRESS:
            printf("BUTTON 4 LONG PRESS\n");
            uiService->handleButton4LongPress();
            break;

        default:
            break;
    }
}

void InteractionService::handleInputEvent(reflow_InputEvent_InputType type, int32_t encoder_steps) {
    Interaction action = Interaction::NONE;

    switch (type) {
        // Encoder events
        case reflow_InputEvent_InputType_ENCODER_UP:
            action = Interaction::ENCODER_UP;
            break;
        case reflow_InputEvent_InputType_ENCODER_DOWN:
            action = Interaction::ENCODER_DOWN;
            break;
        case reflow_InputEvent_InputType_ENCODER_PRESS:
            action = Interaction::ENCODER_PRESS;
            break;
        case reflow_InputEvent_InputType_ENCODER_LONG_PRESS:
            action = Interaction::ENCODER_LONG_PRESS;
            break;

        // Button 1 events
        case reflow_InputEvent_InputType_BUTTON_1_PRESS:
            action = Interaction::BUTTON_1_PRESS;
            break;
        case reflow_InputEvent_InputType_BUTTON_1_LONG_PRESS:
            action = Interaction::BUTTON_1_LONG_PRESS;
            break;

        // Button 2 events
        case reflow_InputEvent_InputType_BUTTON_2_PRESS:
            action = Interaction::BUTTON_2_PRESS;
            break;
        case reflow_InputEvent_InputType_BUTTON_2_LONG_PRESS:
            action = Interaction::BUTTON_2_LONG_PRESS;
            break;

        // Button 3 events
        case reflow_InputEvent_InputType_BUTTON_3_PRESS:
            action = Interaction::BUTTON_3_PRESS;
            break;
        case reflow_InputEvent_InputType_BUTTON_3_LONG_PRESS:
            action = Interaction::BUTTON_3_LONG_PRESS;
            break;

        // Button 4 events
        case reflow_InputEvent_InputType_BUTTON_4_PRESS:
            action = Interaction::BUTTON_4_PRESS;
            break;
        case reflow_InputEvent_InputType_BUTTON_4_LONG_PRESS:
            action = Interaction::BUTTON_4_LONG_PRESS;
            break;

        default:
            break;
    }

    if (action != Interaction::NONE) {
        xQueueSend(interactionQueue, &action, 0);
    }
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

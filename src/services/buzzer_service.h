#pragma once

#include <pb.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "core/service.h"
#include "services/communication_service.h"
#include "library/protos/common.pb.h"
#include "library/protos/controls.pb.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "constants.h"

class BuzzerService : public Service {
public:
    static BuzzerService& getInstance();

    // Initialization
    void init() override;

    // Enable/disable buzzer
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Play different tones
    void playHighTone(uint32_t duration_ms);
    void playMediumTone(uint32_t duration_ms);
    void playLowTone(uint32_t duration_ms);
    void playTone(uint32_t frequency, uint32_t duration_ms);
    void playPattern(uint32_t pattern, uint32_t frequency, uint32_t duration_ms);

private:
    BuzzerService() = default;
    ~BuzzerService() = default;
    BuzzerService(const BuzzerService&) = delete;
    BuzzerService& operator=(const BuzzerService&) = delete;

    // Task handling
    static void buzzerTaskWrapper(void* pvParameters);
    void buzzerTask();

    // Helper method to create and send a buzzer command
    void sendBuzzerCommand(uint32_t pattern, uint32_t frequency, uint32_t duration_ms);

    // Member variables
    bool enabled = true;
    TaskHandle_t taskHandle = nullptr;
}; 
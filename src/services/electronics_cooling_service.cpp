#include "services/electronics_cooling_service.h"
#include "services/sensor_service.h"
#include "constants.h"
#include "globals.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "FreeRTOS.h"
#include "task.h"

#define PWM_FREQUENCY 25000    // 25 kHz suitable for PC fans
#define SYSTEM_CLOCK 125000000 // 125 MHz system clock of the Raspberry Pi Pico
#define CLOCK_DIV 1            // PWM clock divider (should be 1, 2, 4, 8, etc.)

ElectronicsCoolingService& ElectronicsCoolingService::getInstance() {
    static ElectronicsCoolingService instance;
    return instance;
}

ElectronicsCoolingService::ElectronicsCoolingService() {}

void ElectronicsCoolingService::init() {
    uint slice_num = pwm_gpio_to_slice_num(COOLING_FAN_PWM_GPIO);
    gpio_set_function(COOLING_FAN_PWM_GPIO, GPIO_FUNC_PWM);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, CLOCK_DIV);
    pwm_config_set_wrap(&config, calculatePWMWrapValue(PWM_FREQUENCY));

    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(COOLING_FAN_PWM_GPIO, 0); // Ensure fan starts off

    xTaskCreate([](void* arg) {
        static_cast<ElectronicsCoolingService*>(arg)->electronicsCoolingTask();
    }, "ElectronicsCoolinTask", 1024, this, 1, nullptr);
}

uint ElectronicsCoolingService::calculatePWMWrapValue(uint frequency)
{
  return (SYSTEM_CLOCK / (frequency * CLOCK_DIV)) - 1;
}

void ElectronicsCoolingService::electronicsCoolingTask() {
    const TickType_t xDelay = pdMS_TO_TICKS(100); // Smooth ramp
    while (1)
    {
        const SensorState& sensorState = SensorService::getInstance().getState();
        float ssrTemp = sensorState.ssrTemp;

        // Set target fan speed based on SSR temp
        if (ssrTemp < 40.0f) {
            targetFanSpeed = 0;
        } else if (ssrTemp >= 70.0f) {
            targetFanSpeed = 100;
        } else {
            targetFanSpeed = map((long)ssrTemp, 40, 70, 20, 100);
        }

        // Smooth ramp towards target speed
        if (currentFanSpeed != targetFanSpeed)
        {
            if (currentFanSpeed < targetFanSpeed)
                currentFanSpeed += 1;
            else
                currentFanSpeed -= 1;
        }

        // Set PWM level or drive it low
        uint pwmLevel = (currentFanSpeed > 0)
            ? (currentFanSpeed * calculatePWMWrapValue(PWM_FREQUENCY)) / 100
            : 0;

        pwm_set_gpio_level(COOLING_FAN_PWM_GPIO, pwmLevel);

        // Optional: print debug info
        // printf("SSR Temp: %.1f°C | Fan Speed: %d%% | PWM: %u\n", ssrTemp, currentFanSpeed, pwmLevel);

        vTaskDelay(xDelay);
    }
}


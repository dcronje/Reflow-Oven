#include "services/temperature_control_service.h"
#include "services/door_service.h"
#include "constants.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "servo.pio.h"
#include "services/sensor_service.h"
#include <algorithm>

TemperatureControlService& TemperatureControlService::getInstance() {
    static TemperatureControlService instance;
    return instance;
}

TemperatureControlService::TemperatureControlService()
    : targetTemp(0.0f), currentTemp(0.0f),
      heaterPower(0), coolingPower(0),
      lastCoolingChangeTime(0),
      taskHandle(nullptr) {
    state = {};
}

void TemperatureControlService::init() {
    // Initialize heaters
    gpio_init(HEATER_SSR_GPIO);
    gpio_set_dir(HEATER_SSR_GPIO, GPIO_OUT);
    gpio_put(HEATER_SSR_GPIO, 0);

    // Initialize to closed position
    setDoorPosition(0);

    xTaskCreate(controlTaskWrapper, "TempCtrl", 1024, this, 1, &taskHandle);
}

void TemperatureControlService::controlTaskWrapper(void* pvParameters) {
    static_cast<TemperatureControlService*>(pvParameters)->controlTask();
}

void TemperatureControlService::controlTask() {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(HEATER_CONTROL_PERIOD_MS);

    while (true) {
        const SensorState& sensorState = SensorService::getInstance().getState();
        currentTemp = sensorState.currentTemp;

        updateHeaterControl();
        updateCoolingControl();

        vTaskDelayUntil(&lastWakeTime, period);
    }
}

void TemperatureControlService::updateHeaterControl() {
    if (state.hasError || targetTemp == 0.0f) {
        setHeaterPower(0);
        return;
    }

    float error = targetTemp - currentTemp;
    float power = error * TEMPERATURE_CONTROL_KP;
    power = std::clamp(power, 0.0f, 100.0f);
    setHeaterPower(static_cast<uint8_t>(power));

    state.isHeating = (power > 0);
}

void TemperatureControlService::updateCoolingControl() {
    if (state.hasError || targetTemp == 0.0f) {
        setCoolingPower(0);
        return;
    }

    float error = currentTemp - targetTemp;

    if (error <= 0.0f) {
        setCoolingPower(0);
        return;
    }

    uint8_t power = static_cast<uint8_t>((error / 50.0f) * 100.0f);
    power = std::clamp<uint8_t>(power, 0, 100);
    setCoolingPower(power);
}

void TemperatureControlService::setHeaterPower(uint8_t power) {
    heaterPower = power;
    state.output = static_cast<float>(power);
    gpio_put(HEATER_SSR_GPIO, (power > 50));
}

void TemperatureControlService::setCoolingPower(uint8_t power) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now - lastCoolingChangeTime) < MIN_COOLING_CHANGE_INTERVAL) return;

    power = std::clamp<uint8_t>(power, 0, 100);
    coolingPower = power;
    state.coolingPower = power;
    state.isCooling = (power > 0);
    lastCoolingChangeTime = now;

    setDoorPosition(power);
}

void TemperatureControlService::setDoorPosition(uint8_t percent) {
    DoorService::getInstance().setPosition(percent);
}

bool TemperatureControlService::isDoorFullyOpen() const {
    return DoorService::getInstance().isFullyOpen();
}

bool TemperatureControlService::isDoorFullyClosed() const {
    return DoorService::getInstance().isFullyClosed();
}

void TemperatureControlService::setTargetTemperature(float temp) {
    targetTemp = temp;
}

void TemperatureControlService::stopHeating() {
    targetTemp = 0.0f;
    setHeaterPower(0);
    setCoolingPower(100); // Open vents for maximum cooling
}

float TemperatureControlService::getTemperature() const {
    return currentTemp;
}

uint8_t TemperatureControlService::getCoolingPower() const {
    return coolingPower;
}

TemperatureState TemperatureControlService::getState() const {
    return state;
}

float TemperatureControlService::applyCalibration(float rawTemp, size_t thermocoupleIndex) {
    // TODO: Implement temperature calibration
    return rawTemp;
}

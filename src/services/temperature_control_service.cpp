#include "services/temperature_control_service.h"
#include "constants.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "servo.pio.h"
#include "services/sensor_service.h"
#include <algorithm>

TemperatureControlService& TemperatureControlService::getInstance() {
    static TemperatureControlService instance;
    return instance;
}

TemperatureControlService::TemperatureControlService()
    : targetTemp(0.0f), frontTemp(0.0f), backTemp(0.0f),
      frontHeaterPower(0), backHeaterPower(0), coolingPower(0),
      lastCoolingChangeTime(0),
      topLeftVentSm(0), topRightVentSm(0), bottomLeftVentSm(0), bottomRightVentSm(0),
      taskHandle(nullptr) {
    state = {};
}

void TemperatureControlService::init() {
    gpio_init(HEATER_FRONT_SSR_GPIO);
    gpio_set_dir(HEATER_FRONT_SSR_GPIO, GPIO_OUT);
    gpio_put(HEATER_FRONT_SSR_GPIO, 0);

    gpio_init(HEATER_BACK_SSR_GPIO);
    gpio_set_dir(HEATER_BACK_SSR_GPIO, GPIO_OUT);
    gpio_put(HEATER_BACK_SSR_GPIO, 0);

    gpio_set_function(FAN_CONTROL_GPIO, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(FAN_CONTROL_GPIO);
    pwm_set_wrap(slice, 65535);
    pwm_set_enabled(slice, true);

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &servo_program);

    topLeftVentSm = pio_claim_unused_sm(pio, true);
    topRightVentSm = pio_claim_unused_sm(pio, true);
    bottomLeftVentSm = pio_claim_unused_sm(pio, true);
    bottomRightVentSm = pio_claim_unused_sm(pio, true);

    initServoSm(pio, topLeftVentSm, offset, 64.0f, TOP_LEFT_VENT_SERVO_GPIO, SERVO_PERIOD_TICKS);
    initServoSm(pio, topRightVentSm, offset, 64.0f, TOP_RIGHT_VENT_SERVO_GPIO, SERVO_PERIOD_TICKS);
    initServoSm(pio, bottomLeftVentSm, offset, 64.0f, BOTTOM_LEFT_VENT_SERVO_GPIO, SERVO_PERIOD_TICKS);
    initServoSm(pio, bottomRightVentSm, offset, 64.0f, BOTTOM_RIGHT_VENT_SERVO_GPIO, SERVO_PERIOD_TICKS);

    pio_sm_set_enabled(pio, topLeftVentSm, true);
    pio_sm_set_enabled(pio, topRightVentSm, true);
    pio_sm_set_enabled(pio, bottomLeftVentSm, true);
    pio_sm_set_enabled(pio, bottomRightVentSm, true);

    setTopVentsAngle(0);
    setBottomVentsAngle(0);
    setFanSpeed(0);
}

void TemperatureControlService::start() {
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
        frontTemp = sensorState.frontTemp;
        backTemp = sensorState.backTemp;

        updateHeaterControl();
        updateCoolingControl();

        vTaskDelayUntil(&lastWakeTime, period);
    }
}

void TemperatureControlService::updateHeaterControl() {
    if (state.hasError || targetTemp == 0.0f) {
        setFrontHeaterPower(0);
        setBackHeaterPower(0);
        return;
    }

    float frontError = targetTemp - frontTemp;
    float frontPower = frontError * TEMPERATURE_CONTROL_KP;
    frontPower = std::clamp(frontPower, 0.0f, 100.0f);
    setFrontHeaterPower(static_cast<uint8_t>(frontPower));

    float backError = targetTemp - backTemp;
    float backPower = backError * TEMPERATURE_CONTROL_KP;
    backPower = std::clamp(backPower, 0.0f, 100.0f);
    setBackHeaterPower(static_cast<uint8_t>(backPower));

    state.isHeating = (frontPower > 0 || backPower > 0);
}

void TemperatureControlService::updateCoolingControl() {
    if (state.hasError || targetTemp == 0.0f) {
        setCoolingPower(0);
        return;
    }

    float maxTemp = std::max(frontTemp, backTemp);
    float error = maxTemp - targetTemp;

    if (error <= 0.0f) {
        setCoolingPower(0);
        return;
    }

    uint8_t power = static_cast<uint8_t>((error / 50.0f) * 100.0f);
    power = std::clamp<uint8_t>(power, 0, 100);
    setCoolingPower(power);
}

void TemperatureControlService::setFrontHeaterPower(uint8_t power) {
    frontHeaterPower = power;
    state.frontOutput = static_cast<float>(power);
    gpio_put(HEATER_FRONT_SSR_GPIO, (power > 50));
}

void TemperatureControlService::setBackHeaterPower(uint8_t power) {
    backHeaterPower = power;
    state.backOutput = static_cast<float>(power);
    gpio_put(HEATER_BACK_SSR_GPIO, (power > 50));
}

void TemperatureControlService::setCoolingPower(uint8_t power) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if ((now - lastCoolingChangeTime) < MIN_COOLING_CHANGE_INTERVAL) return;

    power = std::clamp<uint8_t>(power, 0, 100);
    coolingPower = power;
    state.coolingPower = power;
    state.isCooling = (power > 0);
    lastCoolingChangeTime = now;

    if (power == 0) {
        setTopVentsAngle(0);
        setBottomVentsAngle(0);
        setFanSpeed(0);
        return;
    }

    if (power <= 40) {
        int angle = (power * 100) / 40;
        setTopVentsAngle(angle);
        setBottomVentsAngle(0);
        setFanSpeed(0);
    } else {
        setTopVentsAngle(100);
        int bottomAngle = std::min((power - 40) * 100 / 10, 100);
        setBottomVentsAngle(bottomAngle);
        int fanPower = std::max((power - 50) * 100 / 50, 0);
        fanPower = std::min(fanPower, 100);
        setFanSpeed(fanPower);
    }
}

void TemperatureControlService::setTopVentsAngle(uint8_t percent) {
    uint pulse = SERVO_MIN_PULSE + ((SERVO_MAX_PULSE - SERVO_MIN_PULSE) * percent) / 100;
    pio_sm_put_blocking(pio0, topLeftVentSm, pulse);
    pio_sm_put_blocking(pio0, topRightVentSm, pulse);
}

void TemperatureControlService::setBottomVentsAngle(uint8_t percent) {
    uint pulse = SERVO_MIN_PULSE + ((SERVO_MAX_PULSE - SERVO_MIN_PULSE) * percent) / 100;
    pio_sm_put_blocking(pio0, bottomLeftVentSm, pulse);
    pio_sm_put_blocking(pio0, bottomRightVentSm, pulse);
}

void TemperatureControlService::setFanSpeed(uint8_t percent) {
    uint16_t level = (65535 * std::clamp<uint8_t>(percent, 0, 100)) / 100;
    pwm_set_gpio_level(FAN_CONTROL_GPIO, level);
}

void TemperatureControlService::setTargetTemperature(float temp) {
    targetTemp = temp;
}

float TemperatureControlService::getFrontTemperature() const {
    return frontTemp;
}

float TemperatureControlService::getBackTemperature() const {
    return backTemp;
}

uint8_t TemperatureControlService::getFrontHeaterPower() const {
    return frontHeaterPower;
}

uint8_t TemperatureControlService::getBackHeaterPower() const {
    return backHeaterPower;
}

uint8_t TemperatureControlService::getCoolingPower() const {
    return coolingPower;
}

uint16_t TemperatureControlService::getFanRPM() const {
    return state.fanRPM;
}

bool TemperatureControlService::isFanRunning() const {
    return state.fanRPM > 0;
}

TemperatureState TemperatureControlService::getState() const {
    return state;
}

void TemperatureControlService::initServoSm(PIO pio, uint sm, uint offset, float clkDiv, uint pin, uint32_t periodTicks) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_exec(pio, sm, pio_encode_set(pio_isr, periodTicks));
    pio_sm_config c = servo_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_clkdiv(&c, clkDiv);
    pio_sm_init(pio, sm, offset, &c);
}

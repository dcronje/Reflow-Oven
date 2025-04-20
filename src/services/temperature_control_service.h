#pragma once

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/pio.h"
#include "types/temperature_state.h"
#include "types/temp_reading.h"

class TemperatureControlService {
public:
    static TemperatureControlService& getInstance();

    void init();
    void setTargetTemperature(float temp);
    void start();

    float getFrontTemperature() const;
    float getBackTemperature() const;
    uint8_t getFrontHeaterPower() const;
    uint8_t getBackHeaterPower() const;
    uint8_t getCoolingPower() const;
    uint16_t getFanRPM() const;
    bool isFanRunning() const;
    TemperatureState getState() const;

    void setFrontHeaterPower(uint8_t power);
    void setBackHeaterPower(uint8_t power);
    void setCoolingPower(uint8_t power);

    void setTopVentsAngle(uint8_t percent);
    void setBottomVentsAngle(uint8_t percent);
    void setFanSpeed(uint8_t percent);

private:
    TemperatureControlService();
    static void controlTaskWrapper(void* pvParameters);
    void controlTask();
    void updateHeaterControl();
    void updateCoolingControl();

    void initServoSm(PIO pio, uint sm, uint offset, float clkDiv, uint pin, uint32_t periodTicks);

    float applyCalibration(float rawTemp, size_t thermocoupleIndex);

    TemperatureState state;

    float targetTemp;
    float frontTemp;
    float backTemp;
    uint8_t frontHeaterPower;
    uint8_t backHeaterPower;
    uint8_t coolingPower;
    uint32_t lastCoolingChangeTime;

    uint topLeftVentSm;
    uint topRightVentSm;
    uint bottomLeftVentSm;
    uint bottomRightVentSm;

    TaskHandle_t taskHandle;
};

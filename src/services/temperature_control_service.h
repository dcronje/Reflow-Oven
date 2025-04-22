#pragma once

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "types/temperature_state.h"
#include "types/temp_reading.h"
#include "constants.h"

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
    TemperatureState getState() const;

    void setFrontHeaterPower(uint8_t power);
    void setBackHeaterPower(uint8_t power);
    void setCoolingPower(uint8_t power);

    void setDoorPosition(uint8_t percent);
    bool isDoorFullyOpen() const;
    bool isDoorFullyClosed() const;

private:
    TemperatureControlService();
    static void controlTaskWrapper(void* pvParameters);
    void controlTask();
    void updateHeaterControl();
    void updateCoolingControl();
    float applyCalibration(float rawTemp, size_t thermocoupleIndex);

    TemperatureState state;

    float targetTemp;
    float frontTemp;
    float backTemp;
    uint8_t frontHeaterPower;
    uint8_t backHeaterPower;
    uint8_t coolingPower;
    uint32_t lastCoolingChangeTime;

    TaskHandle_t taskHandle;
};

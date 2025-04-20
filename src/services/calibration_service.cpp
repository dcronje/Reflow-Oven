// services/calibration_service.cpp

#include "services/calibration_service.h"
#include "services/temperature_control_service.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "constants.h"
#include <string.h>
#include <math.h>

CalibrationService& CalibrationService::getInstance() {
    static CalibrationService instance;
    return instance;
}

CalibrationService::CalibrationService() : taskHandle(nullptr), updateQueue(nullptr), currentMode(Mode::NONE) {
    memset(&data, 0, sizeof(data));
    memset(&state, 0, sizeof(state));
    state.phase = CalibrationPhase::IDLE;
    loadCalibrationData();
}

void CalibrationService::init() {
    xTaskCreate(calibrationTaskWrapper, "CalibSvc", 4096, this, 1, &taskHandle);
}

void CalibrationService::startSensorCalibration() {
    currentMode = Mode::SENSOR;
    calibrationStartTime = get_absolute_time();
    state.phase = CalibrationPhase::TEMPERATURE_CALIBRATION;
    state.hasError = false;
}

void CalibrationService::startThermalCalibration() {
    currentMode = Mode::THERMAL;
    calibrationStartTime = get_absolute_time();
    state.phase = CalibrationPhase::HEATING_CALIBRATION;
    state.hasError = false;
}

void CalibrationService::stopCalibration() {
    currentMode = Mode::NONE;
    state.phase = CalibrationPhase::IDLE;
    TemperatureControlService::getInstance().setFrontHeaterPower(0);
    TemperatureControlService::getInstance().setBackHeaterPower(0);
    TemperatureControlService::getInstance().setCoolingPower(0);
}

bool CalibrationService::isCalibrated() const {
    return data.isCalibrated;
}

const CalibrationData& CalibrationService::getCalibrationData() const {
    return data;
}

const CalibrationState& CalibrationService::getState() const {
    return state;
}

float CalibrationService::getExpectedHeatingRate(float percent) const {
    int i = static_cast<int>(percent / 10.0f) - 1;
    if (i < 0) i = 0;
    if (i > 9) i = 9;
    return data.thermalSummary.heatingRates[i];
}

float CalibrationService::getExpectedCoolingRate(float percent) const {
    int i = static_cast<int>(percent / 10.0f) - 1;
    if (i < 0) i = 0;
    if (i > 9) i = 9;
    return data.thermalSummary.coolingRates[i];
}

void CalibrationService::calibrationTaskWrapper(void* pvParameters) {
    static_cast<CalibrationService*>(pvParameters)->calibrationTask();
}

void CalibrationService::calibrationTask() {
    while (true) {
        switch (currentMode) {
            case Mode::SENSOR:
                runSensorCalibration();
                currentMode = Mode::NONE;
                break;
            case Mode::THERMAL:
                runThermalCalibration();
                currentMode = Mode::NONE;
                break;
            case Mode::NONE:
            default:
                vTaskDelay(pdMS_TO_TICKS(500));
                break;
        }
    }
}

bool CalibrationService::runSensorCalibration() {
    auto& tempService = TemperatureControlService::getInstance();
    absolute_time_t start = get_absolute_time();

    while (absolute_time_diff_us(start, get_absolute_time()) < TEMP_CALIBRATION_TIME_MS * 1000) {
        float front = tempService.getFrontTemperature();
        float back = tempService.getBackTemperature();
        float avg = (front + back) / 2.0f;

        if (fabsf(front - back) > MIN_TEMP_DIFF_FOR_WARNING) {
            displayError("Sensor mismatch");
            return false;
        }

        data.frontSensorOffset = avg - front;
        data.backSensorOffset = avg - back;

        uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(start);
        updateProgress("Sensor Calibration", (float)elapsed / TEMP_CALIBRATION_TIME_MS, avg, TEMP_CALIBRATION_TIME_MS - elapsed);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    data.isCalibrated = true;
    data.lastCalibrationTime = to_ms_since_boot(get_absolute_time());
    state.phase = CalibrationPhase::COMPLETE;
    return saveCalibrationData();
}

bool CalibrationService::runThermalCalibration() {
    auto& tempService = TemperatureControlService::getInstance();
    absolute_time_t start = get_absolute_time();

    for (int power = 10, i = 0; power <= 100; power += 10, ++i) {
        tempService.setFrontHeaterPower(power);
        tempService.setBackHeaterPower(power);

        float tStart = tempService.getFrontTemperature();
        vTaskDelay(pdMS_TO_TICKS(THERMAL_CALIBRATION_TIME_MS));
        float tEnd = tempService.getFrontTemperature();

        data.thermalSummary.heatingRates[i] = (tEnd - tStart) / (THERMAL_CALIBRATION_TIME_MS / 1000.0f);
    }

    for (int fan = 10, i = 0; fan <= 100; fan += 10, ++i) {
        tempService.setCoolingPower(fan);

        float tStart = tempService.getFrontTemperature();
        vTaskDelay(pdMS_TO_TICKS(COOLING_TEST_TIME_MS));
        float tEnd = tempService.getFrontTemperature();

        data.thermalSummary.coolingRates[i] = (tStart - tEnd) / (COOLING_TEST_TIME_MS / 1000.0f);
    }

    tempService.setFrontHeaterPower(0);
    tempService.setBackHeaterPower(0);
    tempService.setCoolingPower(0);

    data.isCalibrated = true;
    data.lastCalibrationTime = to_ms_since_boot(get_absolute_time());
    state.phase = CalibrationPhase::COMPLETE;
    return saveCalibrationData();
}

bool CalibrationService::saveCalibrationData() {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(CALIBRATION_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(CALIBRATION_FLASH_OFFSET, reinterpret_cast<const uint8_t*>(&data), sizeof(CalibrationData));
    restore_interrupts(ints);
    return true;
}

bool CalibrationService::loadCalibrationData() {
    const CalibrationData* fromFlash = reinterpret_cast<const CalibrationData*>(XIP_BASE + CALIBRATION_FLASH_OFFSET);
    if (fromFlash->isCalibrated) {
        data = *fromFlash;
        return true;
    }
    return false;
}

void CalibrationService::updateProgress(const char* label, float progressVal, float current, uint32_t remaining) {
    state.progress = progressVal;
    state.currentTemp = current;
    state.timeRemainingMs = remaining;
}

void CalibrationService::displayError(const char* message) {
    state.phase = CalibrationPhase::ERROR;
    state.hasError = true;
    state.errorMessage = message;
}

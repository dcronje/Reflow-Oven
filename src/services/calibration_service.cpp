// services/calibration_service.cpp

#include "services/calibration_service.h"
#include "services/temperature_control_service.h"
#include "services/sensor_service.h"
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

void CalibrationService::startDoorCalibration() {
    if (currentMode != Mode::NONE) {
        displayError("Calibration already in progress");
        return;
    }

    currentMode = Mode::DOOR;
    state.phase = CalibrationPhase::DOOR_CALIBRATION;
    state.progress = 0.0f;
    state.hasError = false;
    state.errorMessage = nullptr;

    xTaskCreate(calibrationTaskWrapper, "Calibration", 2048, this, 2, &taskHandle);
}

void CalibrationService::stopCalibration() {
    currentMode = Mode::NONE;
    state.phase = CalibrationPhase::IDLE;
    TemperatureControlService::getInstance().setHeaterPower(0);
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

    // Get current temperature
    float currentTemp = TemperatureControlService::getInstance().getTemperature();
    
    // Find the two temperature points to interpolate between
    int lowerTempIdx = 0;
    int upperTempIdx = 0;
    
    for (size_t idx = 0; idx < NUM_TEMP_POINTS; ++idx) {
        if (TEMP_POINTS[idx] > currentTemp) {
            upperTempIdx = idx;
            lowerTempIdx = idx > 0 ? idx - 1 : 0;
            break;
        }
        if (idx == NUM_TEMP_POINTS - 1) {
            lowerTempIdx = upperTempIdx = idx;
        }
    }
    
    // If we're at the same temperature point, no interpolation needed
    if (lowerTempIdx == upperTempIdx) {
        return data.thermalSummary.heatingRates[lowerTempIdx][i];
    }
    
    // Interpolate between the two temperature points
    float lowerTemp = TEMP_POINTS[lowerTempIdx];
    float upperTemp = TEMP_POINTS[upperTempIdx];
    float lowerRate = data.thermalSummary.heatingRates[lowerTempIdx][i];
    float upperRate = data.thermalSummary.heatingRates[upperTempIdx][i];
    
    return lowerRate + (upperRate - lowerRate) * (currentTemp - lowerTemp) / (upperTemp - lowerTemp);
}

float CalibrationService::getExpectedCoolingRate(float percent) const {
    int i = static_cast<int>(percent / 10.0f) - 1;
    if (i < 0) i = 0;
    if (i > 9) i = 9;

    // Get current temperature
    float currentTemp = TemperatureControlService::getInstance().getTemperature();
    
    // Find the two temperature points to interpolate between
    int lowerTempIdx = 0;
    int upperTempIdx = 0;
    
    for (size_t idx = 0; idx < NUM_TEMP_POINTS; ++idx) {
        if (TEMP_POINTS[idx] > currentTemp) {
            upperTempIdx = idx;
            lowerTempIdx = idx > 0 ? idx - 1 : 0;
            break;
        }
        if (idx == NUM_TEMP_POINTS - 1) {
            lowerTempIdx = upperTempIdx = idx;
        }
    }
    
    // If we're at the same temperature point, no interpolation needed
    if (lowerTempIdx == upperTempIdx) {
        return data.thermalSummary.coolingRates[lowerTempIdx][i];
    }
    
    // Interpolate between the two temperature points
    float lowerTemp = TEMP_POINTS[lowerTempIdx];
    float upperTemp = TEMP_POINTS[upperTempIdx];
    float lowerRate = data.thermalSummary.coolingRates[lowerTempIdx][i];
    float upperRate = data.thermalSummary.coolingRates[upperTempIdx][i];
    
    return lowerRate + (upperRate - lowerRate) * (currentTemp - lowerTemp) / (upperTemp - lowerTemp);
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
            case Mode::DOOR:
                runDoorCalibration();
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
    auto& sensorService = SensorService::getInstance();
    absolute_time_t start = get_absolute_time();

    while (absolute_time_diff_us(start, get_absolute_time()) < TEMP_CALIBRATION_TIME_MS * 1000) {
        SensorState state = sensorService.getState();
        float current = state.currentTemp;
        float ambient = state.ambientTemp;

        if (fabsf(current - ambient) > MIN_TEMP_DIFF_FOR_WARNING) {
            displayError("Sensor mismatch");
            return false;
        }

        data.sensorOffset = ambient - current;

        uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(start);
        updateProgress("Sensor Calibration", (float)elapsed / TEMP_CALIBRATION_TIME_MS, current - ambient, TEMP_CALIBRATION_TIME_MS - elapsed);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    data.isCalibrated = true;
    data.lastCalibrationTime = to_ms_since_boot(get_absolute_time());
    state.phase = CalibrationPhase::COMPLETE;
    return saveCalibrationData();
}

bool CalibrationService::runThermalCalibration() {
    auto& tempService = TemperatureControlService::getInstance();

    // Heating calibration phase
    state.phase = CalibrationPhase::HEATING_CALIBRATION;
    
    // For each temperature point
    for (size_t tempIdx = 0; tempIdx < NUM_TEMP_POINTS; ++tempIdx) {
        float targetTemp = TEMP_POINTS[tempIdx];
        char progressMsg[64];
        
        // First, get to the target temperature
        if (tempIdx > 0) {  // Skip for first point (room temperature)
            snprintf(progressMsg, sizeof(progressMsg), "Heating to %d°C", static_cast<int>(targetTemp));
            // Use 50% power to reach target temperature
            tempService.setHeaterPower(50);
            
            // Wait until we're close to target temperature
            while (tempService.getTemperature() < targetTemp - 5.0f) {
                float currentTemp = tempService.getTemperature();
                updateProgress(progressMsg, 0.0f, currentTemp, 0);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            
            // Settle at target temperature
            snprintf(progressMsg, sizeof(progressMsg), "Settling at %d°C", static_cast<int>(targetTemp));
            updateProgress(progressMsg, 0.0f, tempService.getTemperature(), THERMAL_SETTLE_TIME_MS);
            vTaskDelay(pdMS_TO_TICKS(THERMAL_SETTLE_TIME_MS));
        }

        // Now test different power levels at this temperature
        for (int power = 10, i = 0; power <= 100; power += 10, ++i) {
            snprintf(progressMsg, sizeof(progressMsg), "Testing %d%% at %d°C", power, static_cast<int>(targetTemp));
            tempService.setHeaterPower(power);

            // Wait for thermal system to settle
            updateProgress(progressMsg, 0.0f, tempService.getTemperature(), THERMAL_SETTLE_TIME_MS);
            vTaskDelay(pdMS_TO_TICKS(THERMAL_SETTLE_TIME_MS));
            float tStart = tempService.getTemperature();

            uint32_t elapsed = 0;
            const uint32_t interval = 1000; // update every second

            while (elapsed < THERMAL_CALIBRATION_TIME_MS) {
                float currentTemp = tempService.getTemperature();
                float progress = (float)elapsed / THERMAL_CALIBRATION_TIME_MS;
                updateProgress(progressMsg, progress, currentTemp, THERMAL_CALIBRATION_TIME_MS - elapsed);
                vTaskDelay(pdMS_TO_TICKS(interval));
                elapsed += interval;
            }

            float tEnd = tempService.getTemperature();
            data.thermalSummary.heatingRates[tempIdx][i] = (tEnd - tStart) / (THERMAL_CALIBRATION_TIME_MS / 1000.0f);
        }
    }

    // Cooling calibration phase
    state.phase = CalibrationPhase::COOLING_CALIBRATION;
    
    // For each temperature point (in reverse order)
    for (int tempIdx = NUM_TEMP_POINTS - 1; tempIdx >= 0; --tempIdx) {
        float targetTemp = TEMP_POINTS[tempIdx];
        char progressMsg[64];
        
        // First, get to the target temperature
        if (tempIdx < NUM_TEMP_POINTS - 1) {  // Skip for highest temperature point
            snprintf(progressMsg, sizeof(progressMsg), "Cooling to %d°C", static_cast<int>(targetTemp));
            // Use 50% power to reach target temperature
            tempService.setHeaterPower(50);
            
            // Wait until we're close to target temperature
            while (tempService.getTemperature() > targetTemp + 5.0f) {
                float currentTemp = tempService.getTemperature();
                updateProgress(progressMsg, 0.0f, currentTemp, 0);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            
            // Settle at target temperature
            snprintf(progressMsg, sizeof(progressMsg), "Settling at %d°C", static_cast<int>(targetTemp));
            updateProgress(progressMsg, 0.0f, tempService.getTemperature(), THERMAL_SETTLE_TIME_MS);
            vTaskDelay(pdMS_TO_TICKS(THERMAL_SETTLE_TIME_MS));
        }

        // Now test different fan levels at this temperature
        for (int fan = 10, i = 0; fan <= 100; fan += 10, ++i) {
            snprintf(progressMsg, sizeof(progressMsg), "Testing %d%% fan at %d°C", fan, static_cast<int>(targetTemp));
            tempService.setCoolingPower(fan);

            // Wait for thermal system to settle
            updateProgress(progressMsg, 0.0f, tempService.getTemperature(), THERMAL_SETTLE_TIME_MS);
            vTaskDelay(pdMS_TO_TICKS(THERMAL_SETTLE_TIME_MS));
            float tStart = tempService.getTemperature();

            uint32_t elapsed = 0;
            const uint32_t interval = 1000; // update every second

            while (elapsed < COOLING_TEST_TIME_MS) {
                float currentTemp = tempService.getTemperature();
                float progress = (float)elapsed / COOLING_TEST_TIME_MS;
                updateProgress(progressMsg, progress, currentTemp, COOLING_TEST_TIME_MS - elapsed);
                vTaskDelay(pdMS_TO_TICKS(interval));
                elapsed += interval;
            }

            float tEnd = tempService.getTemperature();
            data.thermalSummary.coolingRates[tempIdx][i] = (tStart - tEnd) / (COOLING_TEST_TIME_MS / 1000.0f);
        }
    }

    // Stop any heating/cooling output
    tempService.setHeaterPower(0);
    tempService.setCoolingPower(0);

    data.isCalibrated = true;
    data.lastCalibrationTime = to_ms_since_boot(get_absolute_time());
    state.phase = CalibrationPhase::COMPLETE;
    return saveCalibrationData();
}

bool CalibrationService::runDoorCalibration() {
    // Door calibration is interactive and controlled by the UI
    // This task just monitors for completion
    while (currentMode == Mode::DOOR) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return true;
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

void CalibrationService::setDoorOpenPosition(float position) {
    if (currentMode != Mode::DOOR) {
        displayError("Door calibration not in progress");
        return;
    }

    data.doorCalibration.openPosition = position;
    updateProgress("Setting open position", 0.5f, 0.0f, 0);
}

void CalibrationService::setDoorClosedPosition(float position) {
    if (currentMode != Mode::DOOR) {
        displayError("Door calibration not in progress");
        return;
    }

    data.doorCalibration.closedPosition = position;
    data.doorCalibration.isCalibrated = true;
    updateProgress("Setting closed position", 1.0f, 0.0f, 0);
    stopCalibration();
}

bool CalibrationService::isDoorCalibrated() const {
    return data.doorCalibration.isCalibrated;
}

float CalibrationService::getDoorOpenPosition() const {
    return data.doorCalibration.openPosition;
}

float CalibrationService::getDoorClosedPosition() const {
    return data.doorCalibration.closedPosition;
}

#include "services/sensor_service.h"
#include "constants.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

SensorService& SensorService::getInstance() {
    static SensorService instance;
    return instance;
}

SensorService::SensorService() : sht30(AMBIENT_TEMP_I2C_PORT, SHT30_I2C_ADDR) {
    state = {};
}

void SensorService::init() {
    // Init I2C for SHT30
    i2c_init(AMBIENT_TEMP_I2C_PORT, AMBIENT_TEMP_I2C_BAUDRATE);
    gpio_set_function(AMBIENT_TEMP_I2C_SDA_GPIO, GPIO_FUNC_I2C);
    gpio_set_function(AMBIENT_TEMP_I2C_SCL_GPIO, GPIO_FUNC_I2C);

    // Init SPI chip selects
    gpio_init(THERMOCOUPLE_FRONT_CS_GPIO);
    gpio_set_dir(THERMOCOUPLE_FRONT_CS_GPIO, GPIO_OUT);
    gpio_put(THERMOCOUPLE_FRONT_CS_GPIO, 1);

    gpio_init(THERMOCOUPLE_BACK_CS_GPIO);
    gpio_set_dir(THERMOCOUPLE_BACK_CS_GPIO, GPIO_OUT);
    gpio_put(THERMOCOUPLE_BACK_CS_GPIO, 1);

    sht30.init();

    xTaskCreate([](void* arg) {
        static_cast<SensorService*>(arg)->sensorTask();
    }, "SensorTask", 1024, this, 1, nullptr);
}

void SensorService::sensorTask() {
    while (true) {
        SensorState newState = {};
        uint8_t data[4];

        // Read front thermocouple
        gpio_put(THERMOCOUPLE_FRONT_CS_GPIO, 0);
        spi_read_blocking(THERMOCOUPLE_SPI_PORT, 0, data, 4);
        gpio_put(THERMOCOUPLE_FRONT_CS_GPIO, 1);

        if (data[3] & 0x07) {
            newState.hasError = true;
            newState.lastError = "Front thermocouple error";
        } else {
            uint16_t raw = (data[0] << 8) | data[1];
            raw >>= 2;
            newState.frontTemp = raw * 0.25f;
        }

        // Read back thermocouple
        gpio_put(THERMOCOUPLE_BACK_CS_GPIO, 0);
        spi_read_blocking(THERMOCOUPLE_SPI_PORT, 0, data, 4);
        gpio_put(THERMOCOUPLE_BACK_CS_GPIO, 1);

        if (data[3] & 0x07) {
            newState.hasError = true;
            newState.lastError = "Back thermocouple error";
        } else {
            uint16_t raw = (data[0] << 8) | data[1];
            raw >>= 2;
            newState.backTemp = raw * 0.25f;
        }

        float temp, humidity;
        if (sht30.readAll(&temp, &humidity)) {
            newState.ambientTemp = temp;
            newState.ambientHumidity = humidity;
        }

        state = newState;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

const SensorState& SensorService::getState() const {
    return state;
}

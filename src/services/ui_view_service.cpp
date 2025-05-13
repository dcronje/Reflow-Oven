// ui_view_service.cpp
#include "ui_view_service.h"
#include "constants.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

UIViewService& UIViewService::getInstance() {
    static UIViewService instance;
    return instance;
}

UIViewService::UIViewService() : display(nullptr), taskHandle(nullptr) {}

void UIViewService::init() {
    lv_init();
    initSPI();
    initBacklight();
    initDisplay();

    // Initialize RootView
    rootView = std::make_unique<RootView>();
    rootView->init(display);

    xTaskCreate(updateTaskEntry, "LVGL Update", 2048, this, tskIDLE_PRIORITY + 1, &taskHandle);
}

void UIViewService::initSPI() {
    spi_init(DISPLAY_SPI_PORT, DISPLAY_SPI_FREQ);
    gpio_set_function(DISPLAY_SPI_CLK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_SPI_MOSI_GPIO, GPIO_FUNC_SPI);
    gpio_init(DISPLAY_SPI_CS_GPIO);
    gpio_set_dir(DISPLAY_SPI_CS_GPIO, GPIO_OUT);
    gpio_init(DISPLAY_SPI_DC_GPIO);
    gpio_set_dir(DISPLAY_SPI_DC_GPIO, GPIO_OUT);
    gpio_init(DISPLAY_SPI_RST_GPIO);
    gpio_set_dir(DISPLAY_SPI_RST_GPIO, GPIO_OUT);
}

void UIViewService::initBacklight() {
    gpio_set_function(DISPLAY_BACKLIGHT_GPIO, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(DISPLAY_BACKLIGHT_GPIO);
    channel = pwm_gpio_to_channel(DISPLAY_BACKLIGHT_GPIO);
    pwm_set_wrap(slice_num, DISPLAY_BACKLIGHT_PWM_WRAP);
    pwm_set_chan_level(slice_num, channel, DISPLAY_BACKLIGHT_PWM_WRAP);
    pwm_set_enabled(slice_num, true);
}

void UIViewService::initDisplay() {
    resetDisplay();
    display = lv_st7789_create(
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT,
        LV_LCD_FLAG_NONE,
        st7789_send_cmd,
        st7789_send_color
    );
    lv_st7789_set_gap(display, DISPLAY_X_OFFSET, DISPLAY_Y_OFFSET);
    lv_st7789_set_invert(display, false);
}

void UIViewService::resetDisplay() {
    gpio_put(DISPLAY_SPI_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_put(DISPLAY_SPI_RST_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_put(DISPLAY_SPI_RST_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
}

void UIViewService::setBacklight(float brightness) {
    brightness = brightness < 0.0f ? 0.0f : (brightness > 1.0f ? 1.0f : brightness);
    currentBrightness = brightness;
    uint32_t level = static_cast<uint32_t>(brightness * DISPLAY_BACKLIGHT_PWM_WRAP);
    pwm_set_chan_level(slice_num, channel, level);
}

void UIViewService::updateTaskEntry(void* param) {
    static_cast<UIViewService*>(param)->updateLoop();
}

void UIViewService::updateLoop() {
    const TickType_t delay = pdMS_TO_TICKS(5);
    while (true) {
        lv_timer_handler();
        if (rootView) rootView->update();
        vTaskDelay(delay);
    }
}

void UIViewService::st7789_send_cmd(lv_display_t* disp, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size) {
    gpio_put(DISPLAY_SPI_DC_GPIO, 0);
    gpio_put(DISPLAY_SPI_CS_GPIO, 0);
    spi_write_blocking(DISPLAY_SPI_PORT, cmd, cmd_size);
    if (param && param_size > 0) {
        gpio_put(DISPLAY_SPI_DC_GPIO, 1);
        spi_write_blocking(DISPLAY_SPI_PORT, param, param_size);
    }
    gpio_put(DISPLAY_SPI_CS_GPIO, 1);
}

void UIViewService::st7789_send_color(lv_display_t* disp, const uint8_t* cmd, size_t cmd_size, unsigned char* param, size_t param_size) {
    gpio_put(DISPLAY_SPI_DC_GPIO, 0);
    gpio_put(DISPLAY_SPI_CS_GPIO, 0);
    spi_write_blocking(DISPLAY_SPI_PORT, cmd, cmd_size);
    if (param && param_size > 0) {
        gpio_put(DISPLAY_SPI_DC_GPIO, 1);
        spi_write_blocking(DISPLAY_SPI_PORT, param, param_size);
    }
    gpio_put(DISPLAY_SPI_CS_GPIO, 1);
}

// Forward input events to RootView
void UIViewService::handleEncoderUp() { if (rootView) rootView->handleEncoderUp(); }
void UIViewService::handleEncoderDown() { if (rootView) rootView->handleEncoderDown(); }
void UIViewService::handleEncoderPress() { if (rootView) rootView->handleEncoderPress(); }
void UIViewService::handleEncoderLongPress() { if (rootView) rootView->handleEncoderLongPress(); }

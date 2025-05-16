// Updated UIViewService with full ST7789 initialization, flush_cb, and tick integration

#include "ui_view_service.h"
#include "constants.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "lvgl.h"

UIViewService& UIViewService::getInstance() {
    static UIViewService instance;
    return instance;
}

UIViewService::UIViewService() : display(nullptr), uiTaskHandle(nullptr) {}

void UIViewService::init() {
    lv_init();                             // Initialize LVGL core system
    initSPI();                             // Configure SPI GPIOs and start SPI peripheral
    initBacklight();                       // Setup PWM for backlight control
    initDisplay();                         // Initialize ST7789 display and bind to LVGL

    rootView = std::make_unique<RootView>();
    rootView->init(display);

    // Start LVGL render loop in a separate FreeRTOS task
    xTaskCreate(uiTask, "LVGL Update", 8192, this, tskIDLE_PRIORITY + 1, &uiTaskHandle);
}

void UIViewService::initSPI() {
    spi_init(DISPLAY_SPI_PORT, DISPLAY_SPI_FREQ);
    gpio_set_function(DISPLAY_SPI_CLK_GPIO, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_SPI_MOSI_GPIO, GPIO_FUNC_SPI);

    gpio_init(DISPLAY_SPI_CS_GPIO);
    gpio_set_dir(DISPLAY_SPI_CS_GPIO, GPIO_OUT);
    gpio_put(DISPLAY_SPI_CS_GPIO, 1); // Deselect display

    gpio_init(DISPLAY_SPI_DC_GPIO);
    gpio_set_dir(DISPLAY_SPI_DC_GPIO, GPIO_OUT);
    gpio_put(DISPLAY_SPI_DC_GPIO, 0); // Default to command mode

    gpio_init(DISPLAY_SPI_RST_GPIO);
    gpio_set_dir(DISPLAY_SPI_RST_GPIO, GPIO_OUT);
    gpio_put(DISPLAY_SPI_RST_GPIO, 1); // Display not in reset
}

void UIViewService::initBacklight() {
    gpio_set_function(DISPLAY_BACKLIGHT_GPIO, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(DISPLAY_BACKLIGHT_GPIO);
    channel = pwm_gpio_to_channel(DISPLAY_BACKLIGHT_GPIO);
    pwm_set_wrap(slice_num, DISPLAY_BACKLIGHT_PWM_WRAP);
    pwm_set_chan_level(slice_num, channel, DISPLAY_BACKLIGHT_PWM_WRAP); // Full brightness
    pwm_set_enabled(slice_num, true);
}

void UIViewService::initDisplay() {
    init_display(); // Send ST7789 command initialization sequence

    static lv_color_t buf1[LV_HOR_RES_MAX * 20]; // Partial buffer
    display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    // Configure internal LVGL buffer with partial mode
    lv_display_set_buffers(display, buf1, nullptr, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Set flush callback for pushing pixels to ST7789 over SPI
    lv_display_set_flush_cb(display, [](lv_display_t* disp, const lv_area_t* area, uint8_t* color_p) {
        printf("Flush callback\n");
        UIViewService::getInstance().setAddressWindow(area->x1, area->y1, area->x2, area->y2);
        UIViewService::getInstance().st7789_send_command(ST7789_RAMWR);
        gpio_put(DISPLAY_SPI_DC_GPIO, 1);  // Data mode
        gpio_put(DISPLAY_SPI_CS_GPIO, 0);
        size_t len = lv_area_get_width(area) * lv_area_get_height(area) * 2; // RGB565
        spi_write_blocking(DISPLAY_SPI_PORT, color_p, len);
        gpio_put(DISPLAY_SPI_CS_GPIO, 1);
        if (disp) {
            lv_display_flush_ready(disp);
        }
    });


    // lv_obj_t* scr = lv_scr_act();
    
    // // Create a title label
    // lv_obj_t* title = lv_label_create(scr);
    // lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    // lv_label_set_text(title, "REFLOW OVEN");
    // lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
}

void UIViewService::fillDisplay(uint16_t color) {
    // Set full address window
    setAddressWindow(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    st7789_send_command(ST7789_RAMWR);

    // Convert 16-bit RGB565 color to two bytes
    uint8_t color_high = (color >> 8) & 0xFF;
    uint8_t color_low = color & 0xFF;
    uint8_t color_bytes[2] = {color_high, color_low};

    gpio_put(DISPLAY_SPI_DC_GPIO, 1);  // Data mode
    gpio_put(DISPLAY_SPI_CS_GPIO, 0);  // Select display

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        spi_write_blocking(DISPLAY_SPI_PORT, color_bytes, 2);
    }

    gpio_put(DISPLAY_SPI_CS_GPIO, 1);  // Deselect display
}

void UIViewService::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t caset[4] = { (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF), (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF) };
    uint8_t raset[4] = { (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF), (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF) };
    st7789_send_command(ST7789_CASET);
    st7789_send_data(caset, 4);
    st7789_send_command(ST7789_RASET);
    st7789_send_data(raset, 4);
}

void UIViewService::setBacklight(float brightness) {
    brightness = brightness < 0.0f ? 0.0f : (brightness > 1.0f ? 1.0f : brightness);
    currentBrightness = brightness;
    uint32_t level = static_cast<uint32_t>(brightness * DISPLAY_BACKLIGHT_PWM_WRAP);
    pwm_set_chan_level(slice_num, channel, level);
}

void UIViewService::uiTask(void* param) {
    UIViewService* service = static_cast<UIViewService*>(param);
    TickType_t lastTick = xTaskGetTickCount();
    
    while (true) {
        // Update LVGL tick - this is for animations 
        lv_tick_inc(5);
        
        // Handle LVGL timers and drawing
        lv_timer_handler();
        
        // We no longer need to call rootView->update() periodically
        // Instead, controllers will call invalidateView() when they need to be redrawn
        
        // Delay until next update (just handle LVGL core functionality)
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(5));  // 5ms for smooth LVGL animations
    }
}

void UIViewService::handleEncoderUp() { 
    printf("Encoder up in UIViewService\n");
    if (rootView) rootView->scheduleEncoderUpHandler(5); 
}

void UIViewService::handleEncoderDown() { 
    printf("Encoder down in UIViewService\n");
    if (rootView) rootView->scheduleEncoderDownHandler(5);
}

void UIViewService::handleEncoderPress() { 
    if (rootView) rootView->scheduleEncoderPressHandler(5); 
}

void UIViewService::handleEncoderLongPress() { 
    if (rootView) rootView->scheduleEncoderLongPressHandler(5); 
}

void UIViewService::st7789_send_command(uint8_t cmd) {
    gpio_put(DISPLAY_SPI_DC_GPIO, 0); // Command mode
    gpio_put(DISPLAY_SPI_CS_GPIO, 0);
    spi_write_blocking(DISPLAY_SPI_PORT, &cmd, 1);
    gpio_put(DISPLAY_SPI_CS_GPIO, 1);
}

void UIViewService::st7789_send_data(const uint8_t* data, size_t len) {
    gpio_put(DISPLAY_SPI_DC_GPIO, 1); // Data mode
    gpio_put(DISPLAY_SPI_CS_GPIO, 0);
    spi_write_blocking(DISPLAY_SPI_PORT, data, len);
    gpio_put(DISPLAY_SPI_CS_GPIO, 1);
}

bool UIViewService::init_display() {
    resetDisplay();

    // Software reset and basic wake
    st7789_send_command(ST7789_SWRESET); sleep_ms(150);
    st7789_send_command(ST7789_SLPOUT);  sleep_ms(500);

    // Memory data access control (orientation, RGB order)
    st7789_send_command(ST7789_MADCTL);  uint8_t madctl = ST7789_ROTATION_WORKING_OLD; st7789_send_data(&madctl, 1);

    // Set 16-bit pixel format (RGB565)
    st7789_send_command(ST7789_COLMOD);  uint8_t color_mode = 0x05; st7789_send_data(&color_mode, 1);

    // Porch control settings
    uint8_t b2data[5] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    st7789_send_command(ST7789_PORCTRL); st7789_send_data(b2data, 5);

    // Gate control
    st7789_send_command(ST7789_GCTRL);   uint8_t gc = 0x35; st7789_send_data(&gc, 1);

    // VCOM voltage setting
    st7789_send_command(ST7789_VCOMS);   uint8_t vcom = 0x19; st7789_send_data(&vcom, 1);

    // LCM control
    st7789_send_command(ST7789_LCMCTRL); uint8_t lcm = 0x2C; st7789_send_data(&lcm, 1);

    // Enable VDV and VRH commands
    st7789_send_command(ST7789_VDVVRHEN); uint8_t enable = 0x01; st7789_send_data(&enable, 1);

    // VRH and VDV settings
    st7789_send_command(ST7789_VRHS);    uint8_t vrh = 0x12; st7789_send_data(&vrh, 1);
    st7789_send_command(ST7789_VDVS);    uint8_t vdv = 0x20; st7789_send_data(&vdv, 1);

    // Frame rate control
    st7789_send_command(ST7789_FRCTRL2); uint8_t fr = 0x0F; st7789_send_data(&fr, 1);

    // Power control
    st7789_send_command(ST7789_PWCTRL1); uint8_t pwr[2] = {0xA4, 0xA1}; st7789_send_data(pwr, 2);

    // Positive gamma correction
    st7789_send_command(ST7789_POS_GAM);
    uint8_t e0data[14] = {0xD0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2D};
    st7789_send_data(e0data, 14);

    // Negative gamma correction
    st7789_send_command(ST7789_NEG_GAM);
    uint8_t e1data[14] = {0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0B, 0x16, 0x14, 0x2F, 0x31};
    st7789_send_data(e1data, 14);

    // Turn on inversion, normal mode, and display
    st7789_send_command(ST7789_INVON);  sleep_ms(10);
    st7789_send_command(ST7789_NORON);  sleep_ms(10);
    st7789_send_command(ST7789_DISPON); sleep_ms(100);

    return true;
}

void UIViewService::resetDisplay() {
    gpio_put(DISPLAY_SPI_RST_GPIO, 1); sleep_ms(5);
    gpio_put(DISPLAY_SPI_RST_GPIO, 0); sleep_ms(5);
    gpio_put(DISPLAY_SPI_RST_GPIO, 1); sleep_ms(20);
}

void UIViewService::putDisplayToSleep() {
    st7789_send_command(ST7789_DISPOFF); sleep_ms(10);
    st7789_send_command(ST7789_SLPIN);   sleep_ms(120);
}

void UIViewService::wakeDisplayFromSleep() {
    st7789_send_command(ST7789_SLPOUT);  sleep_ms(120);
    st7789_send_command(ST7789_DISPON);  sleep_ms(10);
}

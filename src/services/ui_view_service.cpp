// Updated UIViewService with full ST7789 initialization, flush_cb, and tick integration

#include "ui_view_service.h"
#include "ui/root_view.h"
#include "constants.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "lvgl.h"
#include "core/message_event_bus.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "generated/protos/controls.pb.h"
#include "core/message_event_adapter.h"

UIViewService& UIViewService::getInstance() {
    static UIViewService instance;
    return instance;
}

UIViewService::UIViewService() 
    : display(nullptr), 
      uiTaskHandle(nullptr),
      inputEventHandler(nullptr) {}

void UIViewService::init() {
    printf("Initializing UIViewService\n");
    // Initialize Display
    initDisplay();
    printf("Display initialized\n");

    // Initialize LVGL (if not already)
    if (display == nullptr) {
        // Create a buffer for LVGL
        static lv_disp_draw_buf_t draw_buf;
        static lv_color_t buf1[DISPLAY_WIDTH * 32];
        static lv_color_t buf2[DISPLAY_WIDTH * 32];
        lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISPLAY_WIDTH * 32);

        // Create LVGL display with our flush callback
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.hor_res = DISPLAY_WIDTH;
        disp_drv.ver_res = DISPLAY_HEIGHT;
        disp_drv.flush_cb = flush_cb; // Custom callback must be defined elsewhere
        disp_drv.draw_buf = &draw_buf;
        display = lv_disp_drv_register(&disp_drv);
    }
    
    // Create UI task
    xTaskCreate(
        uiTask,
        "uiTask",
        2048,
        this,
        1,
        &uiTaskHandle
    );
    
    // Register with MessageEventBus
    MessageEventBus::getInstance().registerHandler(this);
    
    // Register for specific message types
    MessageEventBus::getInstance().registerTypeHandler(
        static_cast<uint32_t>(MessageType::CONTROLS), this);
}

void UIViewService::registerInputEventHandler(InputEventsInterface* handler) {
    inputEventHandler = handler;
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

    static lv_color_t buf1[LV_HOR_RES_MAX * 40]; // Doubled buffer size from 20 to 40 lines
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
        lv_tick_inc(1);
        
        // Handle LVGL timers and drawing
        lv_timer_handler();
        
        // We no longer need to call rootView->update() periodically
        // Instead, controllers will call invalidateView() when they need to be redrawn
        
        // Delay until next update (just handle LVGL core functionality)
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(1));  // 1ms for smoother LVGL animations
    }
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

// Schedule encoder event using LVGL timers for safe UI interaction
void UIViewService::scheduleEncoderEvent(EncoderEvent event, uint32_t delayMs) {
    switch (event) {
        case EncoderEvent::UP:
            lv_timer_create(encoderUpCallback, delayMs, this);
            break;
        case EncoderEvent::DOWN:
            lv_timer_create(encoderDownCallback, delayMs, this);
            break;
        default:
            break;
    }
}

// Schedule button event using LVGL timers for safe UI interaction
void UIViewService::scheduleButtonEvent(int buttonId, bool isLongPress, uint32_t delayMs) {
    // Store the button information in a struct and pass it to the timer
    ButtonState* state = new ButtonState{buttonId, isLongPress};
    
    if (isLongPress) {
        lv_timer_t* timer = lv_timer_create(buttonLongPressCallback, delayMs, this);
        lv_timer_set_user_data(timer, state);
    } else {
        lv_timer_t* timer = lv_timer_create(buttonPressCallback, delayMs, this);
        lv_timer_set_user_data(timer, state);
    }
}

// Timer callbacks for safe LVGL interaction
void UIViewService::encoderUpCallback(struct _lv_timer_t* timer) {
    UIViewService* service = static_cast<UIViewService*>(lv_timer_get_user_data(timer));
    if (service && service->inputEventHandler) {
        service->inputEventHandler->handleEncoderUp();
    }
    lv_timer_del(timer);
}

void UIViewService::encoderDownCallback(struct _lv_timer_t* timer) {
    UIViewService* service = static_cast<UIViewService*>(lv_timer_get_user_data(timer));
    if (service && service->inputEventHandler) {
        service->inputEventHandler->handleEncoderDown();
    }
    lv_timer_del(timer);
}

void UIViewService::buttonPressCallback(struct _lv_timer_t* timer) {
    UIViewService* service = static_cast<UIViewService*>(timer->user_data);
    ButtonState* state = static_cast<ButtonState*>(timer->user_data);
    
    if (service && service->inputEventHandler && state) {
        service->inputEventHandler->handleButtonPress(state->buttonId);
        delete state; // Clean up the allocated state
    }
    lv_timer_del(timer);
}

void UIViewService::buttonLongPressCallback(struct _lv_timer_t* timer) {
    UIViewService* service = static_cast<UIViewService*>(timer->user_data);
    ButtonState* state = static_cast<ButtonState*>(timer->user_data);
    
    if (service && service->inputEventHandler && state) {
        service->inputEventHandler->handleButtonLongPress(state->buttonId);
        delete state; // Clean up the allocated state
    }
    lv_timer_del(timer);
}

// Handler methods called from message processing
void UIViewService::handleEncoderUp() {
    printf("Encoder UP received\n");
    scheduleEncoderEvent(EncoderEvent::UP, 5);
}

void UIViewService::handleEncoderDown() {
    printf("Encoder DOWN received\n");
    scheduleEncoderEvent(EncoderEvent::DOWN, 5);
}

void UIViewService::handleButtonPress(int buttonId) {
    printf("Button PRESS received: %d\n", buttonId);
    scheduleButtonEvent(buttonId, false, 5);
}

void UIViewService::handleButtonLongPress(int buttonId) {
    printf("Button LONG PRESS received: %d\n", buttonId);
    scheduleButtonEvent(buttonId, true, 5);
}

// MessageHandler interface implementation
bool UIViewService::processMessage(const void* data, size_t size) {
    if (data == nullptr || size == 0) {
        return false;
    }
    
    // Process the message as a control message
    reflow_ControlsMessage message = reflow_ControlsMessage_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(static_cast<const pb_byte_t*>(data), size);
    
    if (!pb_decode(&stream, reflow_ControlsMessage_fields, &message)) {
        // Failed to decode as a controls message
        return false;
    }
    
    // Process input events
    if (message.which_payload == reflow_ControlsMessage_input_event_tag) {
        auto& input = message.payload.input_event;
        
        // Process input events based on type
        switch (input.type) {
            case reflow_InputEvent_InputType_ENCODER_UP:
                handleEncoderUp();
                return true;
            
            case reflow_InputEvent_InputType_ENCODER_DOWN:
                handleEncoderDown();
                return true;
            
            case reflow_InputEvent_InputType_ENCODER_PRESS:
                // Treat encoder press as ENCODER_BUTTON press
                handleButtonPress(ButtonId::ENCODER_BUTTON);
                return true;
                
            case reflow_InputEvent_InputType_ENCODER_LONG_PRESS:
                // Treat encoder long press as ENCODER_BUTTON long press
                handleButtonLongPress(ButtonId::ENCODER_BUTTON);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_1_PRESS:
                handleButtonPress(ButtonId::BUTTON_1);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_1_LONG_PRESS:
                handleButtonLongPress(ButtonId::BUTTON_1);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_2_PRESS:
                handleButtonPress(ButtonId::BUTTON_2);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_2_LONG_PRESS:
                handleButtonLongPress(ButtonId::BUTTON_2);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_3_PRESS:
                handleButtonPress(ButtonId::BUTTON_3);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_3_LONG_PRESS:
                handleButtonLongPress(ButtonId::BUTTON_3);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_4_PRESS:
                handleButtonPress(ButtonId::BUTTON_4);
                return true;
                
            case reflow_InputEvent_InputType_BUTTON_4_LONG_PRESS:
                handleButtonLongPress(ButtonId::BUTTON_4);
                return true;
                
            default:
                return false;
        }
    }
    
    return false;
}

bool UIViewService::processMessage(const std::string& serialized) {
    if (serialized.empty()) {
        return false;
    }
    return processMessage(serialized.data(), serialized.size());
}

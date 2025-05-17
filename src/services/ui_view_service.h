#pragma once

#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <memory>
#include "ui/root_view.h"
#include "core/encoder_events_interface.h"

// System function commands
#define ST7789_NOP      0x00  // No Operation
#define ST7789_SWRESET  0x01  // Software Reset
#define ST7789_RDDID    0x04  // Read Display ID
#define ST7789_RDDST    0x09  // Read Display Status

// Sleep mode and display on/off
#define ST7789_SLPIN    0x10  // Enter Sleep Mode
#define ST7789_SLPOUT   0x11  // Exit Sleep Mode
#define ST7789_PTLON    0x12  // Partial Mode ON
#define ST7789_NORON    0x13  // Normal Display Mode ON

// Display inversion
#define ST7789_INVOFF   0x20  // Display Inversion OFF
#define ST7789_INVON    0x21  // Display Inversion ON

// Display on/off
#define ST7789_DISPOFF  0x28  // Display OFF
#define ST7789_DISPON   0x29  // Display ON

// Column and row addressing
#define ST7789_CASET    0x2A  // Column Address Set
#define ST7789_RASET    0x2B  // Row Address Set
#define ST7789_RAMWR    0x2C  // Memory Write
#define ST7789_RAMRD    0x2E  // Memory Read

// Tearing effect
#define ST7789_TEOFF    0x34  // Tearing Effect Line OFF
#define ST7789_TEON     0x35  // Tearing Effect Line ON

// Memory and pixel format
#define ST7789_MADCTL   0x36  // Memory Access Control (rotation, RGB/BGR, etc.)
#define ST7789_COLMOD   0x3A  // Interface Pixel Format (e.g., 0x05 = 16-bit/pixel)

// Brightness control (if supported)
#define ST7789_WRDISBV  0x51  // Write Display Brightness
#define ST7789_WRCTRLD  0x53  // Write Control Display

// Gamma function
#define ST7789_GAMSET   0x26  // Gamma Curve Select
#define ST7789_POS_GAM  0xE0  // Positive Gamma Correction
#define ST7789_NEG_GAM  0xE1  // Negative Gamma Correction

// Frame rate and power control
#define ST7789_PORCTRL  0xB2  // Porch Setting
#define ST7789_GCTRL    0xB7  // Gate Control
#define ST7789_VCOMS    0xBB  // VCOM Setting
#define ST7789_LCMCTRL  0xC0  // LCM Control
#define ST7789_VDVVRHEN 0xC2  // VDV and VRH Command Enable
#define ST7789_VRHS     0xC3  // VRH Set
#define ST7789_VDVS     0xC4  // VDV Set
#define ST7789_FRCTRL2  0xC6  // Frame Rate Control in Normal Mode
#define ST7789_PWCTRL1  0xD0  // Power Control 1

// ST7789 MADCTL (Memory Access Control) values for various orientations
#define ST7789_MADCTL_MY    0x80  // Row address order (vertical flip)
#define ST7789_MADCTL_MX    0x40  // Column address order (horizontal flip)
#define ST7789_MADCTL_MV    0x20  // Row/Column exchange (rotation)
#define ST7789_MADCTL_ML    0x10  // Scan order (usually not used)
#define ST7789_MADCTL_BGR   0x08  // BGR color order (set for most ST7789 modules)
#define ST7789_MADCTL_MH    0x04  // Horizontal refresh order (rarely used)

// Common predefined orientation settings
#define ST7789_ROTATION_0       0x00  // Portrait:     Top left origin, RGB
#define ST7789_ROTATION_90      0x20  // Landscape:    90° rotation (MV)
#define ST7789_ROTATION_90_MX   0x60  // Landscape:    90° + mirror X (MV | MX)
#define ST7789_ROTATION_270     0xA0  // Landscape:    270° (MV | MY)
#define ST7789_ROTATION_MIRROR  0xC0  // Portrait:     Horizontal + Vertical flip (MX | MY)
#define ST7789_ROTATION_90_FULL 0xE0  // Landscape:    90° + mirror X + mirror Y (MV | MX | MY)

// With BGR (for most ST7789 screens)
#define ST7789_ROTATION_0_BGR       (ST7789_ROTATION_0       | ST7789_MADCTL_BGR)
#define ST7789_ROTATION_90_BGR      (ST7789_ROTATION_90      | ST7789_MADCTL_BGR)
#define ST7789_ROTATION_90_MX_BGR   (ST7789_ROTATION_90_MX   | ST7789_MADCTL_BGR)
#define ST7789_ROTATION_270_BGR     (ST7789_ROTATION_270     | ST7789_MADCTL_BGR)
#define ST7789_ROTATION_MIRROR_BGR  (ST7789_ROTATION_MIRROR  | ST7789_MADCTL_BGR)
#define ST7789_ROTATION_90_FULL_BGR (ST7789_ROTATION_90_FULL | ST7789_MADCTL_BGR)
#define ST7789_ROTATION_WORKING_OLD (ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_ML)
#define ST7789_ROTATION_WORKING (ST7789_MADCTL_MX | ST7789_MADCTL_MV)
#define ST7789_ROTATION_WORKING_BGR (ST7789_ROTATION_WORKING | ST7789_MADCTL_BGR)

// Encoder event type enum
enum class EncoderEvent {
    UP,
    DOWN,
    PRESS,
    LONG_PRESS
};

class UIViewService {
public:
    static UIViewService& getInstance();

    void init();
    void initSPI();
    void initBacklight();
    bool init_display();
    void setBacklight(float brightness);
    
    // Add display accessor
    lv_display_t* getDisplay() const { return display; }

    void putDisplayToSleep();
    void wakeDisplayFromSleep();
    void fillDisplay(uint16_t color);
    
    // Register a view that implements EncoderEventsInterface
    void registerEncoderEventHandler(EncoderEventsInterface* handler);

    // Event forwarding - these are called from InteractionService
    void handleEncoderUp();
    void handleEncoderDown();
    void handleEncoderPress();
    void handleEncoderLongPress();
    
    // Timer callbacks for safe LVGL interaction
    static void encoderUpCallback(struct _lv_timer_t* timer);
    static void encoderDownCallback(struct _lv_timer_t* timer);
    static void encoderPressCallback(struct _lv_timer_t* timer);
    static void encoderLongPressCallback(struct _lv_timer_t* timer);

private:
    UIViewService();
    static void uiTask(void* param);
    
    void initDisplay();
    void resetDisplay();
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    void st7789_send_command(uint8_t cmd);
    void st7789_send_data(const uint8_t* data, size_t len);

    TaskHandle_t uiTaskHandle;
    
    // Schedule an encoder event with timer to ensure LVGL safety
    void scheduleEncoderEvent(EncoderEventsInterface::EventType event, uint32_t delayMs);
    
    lv_display_t* display;
    
    // Store reference to the encoder event handler (usually RootView)
    EncoderEventsInterface* encoderEventHandler;

    // PWM/backlight
    int slice_num;
    int channel;
    float currentBrightness = 1.0f;
};

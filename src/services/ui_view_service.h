// ui_view_service.h
#pragma once

#include "lvgl.h"
#include "FreeRTOS.h"
#include "task.h"
#include <memory>
#include "ui/root_view.h"

class UIViewService {
public:
    static UIViewService& getInstance();

    void init();
    void setBacklight(float brightness);

    // Event forwarding
    void handleEncoderUp();
    void handleEncoderDown();
    void handleEncoderPress();
    void handleEncoderLongPress();

private:
    UIViewService();
    void update();
    static void updateTaskEntry(void* param);
    void updateLoop();

    void initDisplay();
    void initBacklight();
    void initSPI();
    void resetDisplay();
    static void st7789_send_cmd(lv_display_t* disp, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size);
    static void st7789_send_color(lv_display_t* disp, const uint8_t* cmd, size_t cmd_size, unsigned char* param, size_t param_size);

    TaskHandle_t taskHandle;
    lv_display_t* display;
    std::unique_ptr<RootView> rootView;

    // PWM/backlight
    int slice_num;
    int channel;
    float currentBrightness = 1.0f;
};

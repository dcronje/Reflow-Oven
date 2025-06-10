#pragma once

#include "core/controller.h"
#include "ui/cyberpunk_layout.h"
#include <vector>
#include <utility>

class HomeController : public Controller {
public:
    static HomeController& getInstance();
    ~HomeController() override;

    void buildView(lv_obj_t* parent) override;
    void viewDidLoad() override;
    void viewWillAppear() override;
    void viewDidAppear() override;
    void viewWillDisappear() override;
    void viewDidDisappear() override;
    void viewWillUnload() override;

    // Event handlers
    void onButton1Press() override;
    void onButton2Press() override;
    void onButton3Press() override;
    void onEncoderPress() override;
    void onEncoderUp() override;
    void onEncoderDown() override;
    void onEncoderLongPress() override;

    // UI action methods
    void openMenu();
    void startReflow();
    void stopReflow();

private:
    HomeController() = default;
    HomeController(const HomeController&) = delete;
    HomeController& operator=(const HomeController&) = delete;

    void updateTags();
    void updateStatusDisplay();
    void toggleDoor();
    void toggleLights();
    void selectProfile();

    // Timer management
    void startUpdateTimer();
    void stopUpdateTimer();
    static void updateTimerCallback(lv_timer_t* timer);
    void periodicUpdate();

    CyberpunkLayout* layout = nullptr;
    lv_timer_t* updateTimer = nullptr;
    bool lightsOn = false;
    bool encoderPressed = false;
    
    // Button press feedback duration
    static constexpr uint32_t BUTTON_PRESS_DURATION = 200;
    static constexpr uint32_t ENCODER_PRESS_DURATION = 150;

    // Status display members
    lv_obj_t* statusDisplay = nullptr;
    std::vector<std::pair<lv_obj_t*, lv_obj_t*>> statusLabels;  // pairs of (label, value) objects
};

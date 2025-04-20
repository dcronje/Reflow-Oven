#pragma once

#include "base_controller.h"
#include "models/reflow_model.h"

enum class ReflowState {
    PRECHECK,
    RUNNING,
    COMPLETE
};

class ReflowController : public BaseController {
public:
    static ReflowController& getInstance();

    void init() override;
    void registerViews(UIViewService& viewService) override;
    void start() override;
    void stop() override;

    void requestStart();          // Called by UI when user selects reflow
    void confirmStart();          // Called by UI when user presses "start" (after precheck)
    void cancel();

    ReflowModel& getModel();
    ReflowState getState() const;
    float getElapsedMsInStep() const;
    float getCurrentTargetTemp() const;

    void setCurrentTemp(float tempC); // Sensor-driven
    float getCurrentTemp() const;

protected:
    void run() override;

private:
    ReflowController() = default;

    ReflowModel model;
    ReflowState state = ReflowState::PRECHECK;

    bool reflowRequested = false;
    bool coolingRequired = false;
    float currentTemp = 25.0f;
    uint32_t stepStartTimeMs = 0;
};

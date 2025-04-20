#include "controllers/reflow_controller.h"
#include "services/ui_view_service.h"
#include "ui/views/reflow_precheck_view.h"
#include "ui/views/reflow_process_view.h"
#include "ui/views/reflow_summary_view.h"

#include "pico/time.h"

ReflowController& ReflowController::getInstance() {
    static ReflowController instance;
    return instance;
}

void ReflowController::init() {
    model.setActiveCurve({
        "LEAD-FREE",
        50.0f,
        {
            {"Preheat", 150, 60000},
            {"Soak", 180, 40000},
            {"Reflow", 245, 30000},
            {"Cool", 50, 60000}
        }
    });
}

void ReflowController::registerViews(UIViewService& ui) {
    ui.registerView("reflow-precheck", std::make_unique<ReflowPrecheckView>());
    ui.registerView("reflow-process", std::make_unique<ReflowProcessView>());
    ui.registerView("reflow-summary", std::make_unique<ReflowSummaryView>());
}

void ReflowController::start() {
    requestStart();
}

void ReflowController::requestStart() {
    state = ReflowState::PRECHECK;
    UIViewService::getInstance().showView("reflow-precheck");
}

void ReflowController::confirmStart() {
    state = ReflowState::RUNNING;
    model.resetProgress();
    stepStartTimeMs = to_ms_since_boot(get_absolute_time());
    createTask("ReflowTask", 2048, 2);
    UIViewService::getInstance().showView("reflow-process");
}

void ReflowController::cancel() {
    stop();
    UIViewService::getInstance().showView("main-menu");
}

void ReflowController::stop() {
    state = ReflowState::COMPLETE;
    reflowRequested = false;
    vTaskDelete(nullptr);
    UIViewService::getInstance().showView("reflow-summary");
}

ReflowModel& ReflowController::getModel() {
    return model;
}

ReflowState ReflowController::getState() const {
    return state;
}

void ReflowController::setCurrentTemp(float tempC) {
    currentTemp = tempC;
}

float ReflowController::getCurrentTemp() const {
    return currentTemp;
}

float ReflowController::getElapsedMsInStep() const {
    return to_ms_since_boot(get_absolute_time()) - stepStartTimeMs;
}

float ReflowController::getCurrentTargetTemp() const {
    const auto& step = model.getCurrentStep();
    float elapsed = getElapsedMsInStep();
    float ratio = elapsed / step.durationMs;
    if (ratio > 1.0f) ratio = 1.0f;

    float previous = (model.getCurrentStepIndex() == 0)
                     ? currentTemp
                     : model.getActiveCurve().steps[model.getCurrentStepIndex() - 1].targetTempC;

    return previous + (step.targetTempC - previous) * ratio;
}

void ReflowController::run() {
    while (!model.isComplete()) {
        vTaskDelay(pdMS_TO_TICKS(250));

        float elapsed = getElapsedMsInStep();
        if (elapsed >= model.getCurrentStep().durationMs) {
            model.advanceStep();
            stepStartTimeMs = to_ms_since_boot(get_absolute_time());
        }
    }

    stop();
}

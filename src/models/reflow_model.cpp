#include "reflow_model.h"

void ReflowModel::setActiveCurve(const ReflowCurve& curve) {
    activeCurve = curve;
    currentStepIndex = 0;
}

const ReflowCurve& ReflowModel::getActiveCurve() const {
    return activeCurve;
}

void ReflowModel::resetProgress() {
    currentStepIndex = 0;
}

void ReflowModel::advanceStep() {
    if (currentStepIndex < (int)activeCurve.steps.size() - 1)
        currentStepIndex++;
}

int ReflowModel::getCurrentStepIndex() const {
    return currentStepIndex;
}

const ReflowStep& ReflowModel::getCurrentStep() const {
    return activeCurve.steps.at(currentStepIndex);
}

bool ReflowModel::isComplete() const {
    return currentStepIndex >= static_cast<int>(activeCurve.steps.size());
}

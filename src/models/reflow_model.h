#pragma once

#include <vector>
#include <string>
#include <stdint.h>

struct ReflowStep {
    std::string label;         // e.g., "Preheat", "Soak"
    float targetTempC;
    uint32_t durationMs;
};

struct ReflowCurve {
    std::string name;
    float minimumStartTempC;             // Minimum oven temp to begin reflow
    std::vector<ReflowStep> steps;
};

class ReflowModel {
public:
    void setActiveCurve(const ReflowCurve& curve);
    const ReflowCurve& getActiveCurve() const;

    void resetProgress();
    void advanceStep();
    int getCurrentStepIndex() const;
    const ReflowStep& getCurrentStep() const;
    bool isComplete() const;

private:
    ReflowCurve activeCurve;
    int currentStepIndex = 0;
};

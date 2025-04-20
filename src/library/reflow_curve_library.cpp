#include "library/reflow_curve_library.h"

std::vector<ReflowCurve> ReflowCurveLibrary::builtInCurves;
bool ReflowCurveLibrary::initialized = false;

void ReflowCurveLibrary::initialize() {
    if (initialized) return;
    initialized = true;

    builtInCurves = {
        {
            "Lead-Free (SAC305)",
            50.0f,
            {
                {"Preheat", 150.0f, 60000},
                {"Soak", 180.0f, 90000},
                {"Reflow", 245.0f, 30000},
                {"Cooldown", 50.0f, 60000}
            }
        },
        {
            "Leaded (Sn63Pb37)",
            50.0f,
            {
                {"Preheat", 140.0f, 60000},
                {"Soak", 160.0f, 90000},
                {"Reflow", 215.0f, 30000},
                {"Cooldown", 50.0f, 60000}
            }
        },
        {
            "Custom Profile", // Placeholder
            50.0f,
            {
                {"Preheat", 120.0f, 30000},
                {"Ramp Up", 200.0f, 60000},
                {"Reflow", 230.0f, 30000},
                {"Cooldown", 50.0f, 60000}
            }
        }
    };
}

const std::vector<ReflowCurve>& ReflowCurveLibrary::getBuiltInCurves() {
    initialize();
    return builtInCurves;
}

std::optional<ReflowCurve> ReflowCurveLibrary::getCurveByName(const std::string& name) {
    initialize();
    for (const auto& curve : builtInCurves) {
        if (curve.name == name) {
            return curve;
        }
    }
    return std::nullopt;
}
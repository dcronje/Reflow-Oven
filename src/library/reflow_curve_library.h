#pragma once

#include "models/reflow_model.h"
#include <vector>
#include <optional>

class ReflowCurveLibrary {
public:
    static const std::vector<ReflowCurve>& getBuiltInCurves();
    static std::optional<ReflowCurve> getCurveByName(const std::string& name);

private:
    static std::vector<ReflowCurve> builtInCurves;
    static void initialize();
    static bool initialized;
};
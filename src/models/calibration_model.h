#pragma once

#include "types/calibration_data.h"
#include "types/calibration_state.h"

class CalibrationModel {
public:
    CalibrationModel();
    
    // Getters for calibration state
    bool isCalibrated() const;
    const CalibrationData& getCalibrationData() const;
    const CalibrationState& getState() const;
    
    // Update methods
    void updateFromService();
    
private:
    CalibrationData data;
    CalibrationState state;
}; 
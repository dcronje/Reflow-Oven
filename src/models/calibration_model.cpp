#include "models/calibration_model.h"
#include "services/calibration_service.h"

CalibrationModel::CalibrationModel() {
    updateFromService();
}

bool CalibrationModel::isCalibrated() const {
    return data.isCalibrated;
}

const CalibrationData& CalibrationModel::getCalibrationData() const {
    return data;
}

const CalibrationState& CalibrationModel::getState() const {
    return state;
}

void CalibrationModel::updateFromService() {
    auto& service = CalibrationService::getInstance();
    data = service.getCalibrationData();
    state = service.getState();
} 
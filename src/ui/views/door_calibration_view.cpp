#include "door_calibration_view.h"
#include "controllers/calibration_controller.h"

void DoorCalibrationView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
    CalibrationController::getInstance().startDoorCalibration();
}

void DoorCalibrationView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();
    
    // Draw title
    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, "Door Calibration", STYLE_BOLD);
    
    // Draw step indicator
    drawStepIndicator(static_cast<int>(currentStep));
    
    // Draw current position
    float currentPosition = CalibrationController::getInstance().getDoorCurrentPosition();
    drawCurrentPosition(currentPosition);
    
    // Draw progress bar
    float progress = 0.0f;
    switch (currentStep) {
        case CalibrationStep::OPEN:
            progress = 0.33f;
            break;
        case CalibrationStep::CLOSED:
            progress = 0.66f;
            break;
        case CalibrationStep::COMPLETE:
            progress = 1.0f;
            break;
    }
    drawProgressBar(0, 50, 96, 8, progress);
    
    // Draw instructions
    canvas.setColor(WHITE);
    switch (currentStep) {
        case CalibrationStep::OPEN:
            canvas.printFixed(0, 20, "1. Move door to open", STYLE_NORMAL);
            canvas.printFixed(0, 30, "2. Press to set", STYLE_NORMAL);
            break;
        case CalibrationStep::CLOSED:
            canvas.printFixed(0, 20, "1. Move door to closed", STYLE_NORMAL);
            canvas.printFixed(0, 30, "2. Press to set", STYLE_NORMAL);
            break;
        case CalibrationStep::COMPLETE:
            canvas.printFixed(0, 20, "Calibration complete!", STYLE_NORMAL);
            canvas.printFixed(0, 30, "Press to continue", STYLE_NORMAL);
            break;
    }
    
    display.drawCanvas(0, 0, canvas);
}

void DoorCalibrationView::handleEncoderUp() {
    CalibrationController::getInstance().moveDoorUp();
}

void DoorCalibrationView::handleEncoderDown() {
    CalibrationController::getInstance().moveDoorDown();
}

void DoorCalibrationView::handleEncoderPress() {
    float currentPosition = CalibrationController::getInstance().getDoorCurrentPosition();
    
    switch (currentStep) {
        case CalibrationStep::OPEN:
            CalibrationController::getInstance().setDoorOpenPosition(currentPosition);
            currentStep = CalibrationStep::CLOSED;
            break;
        case CalibrationStep::CLOSED:
            CalibrationController::getInstance().setDoorClosedPosition(currentPosition);
            currentStep = CalibrationStep::COMPLETE;
            break;
        case CalibrationStep::COMPLETE:
            CalibrationController::getInstance().returnToCalibrationMenu();
            break;
    }
}

void DoorCalibrationView::handleEncoderLongPress() {
    // No action on long press
}

void DoorCalibrationView::drawProgressBar(int x, int y, int width, int height, float progress) {
    // Draw background
    canvas.setColor(GREY);
    canvas.fillRect(x, y, width, height);
    
    // Draw progress
    canvas.setColor(GREEN);
    int progressWidth = static_cast<int>(width * progress);
    canvas.fillRect(x, y, progressWidth, height);
}

void DoorCalibrationView::drawStepIndicator(int step) {
    canvas.setColor(WHITE);
    char stepText[16];
    snprintf(stepText, sizeof(stepText), "Step %d/3", step + 1);
    canvas.printFixed(0, 12, stepText, STYLE_NORMAL);
}

void DoorCalibrationView::drawCurrentPosition(float position) {
    canvas.setColor(WHITE);
    char posText[16];
    snprintf(posText, sizeof(posText), "Pos: %.1f%%", position);
    canvas.printFixed(0, 40, posText, STYLE_NORMAL);
} 
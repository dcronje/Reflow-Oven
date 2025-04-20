#include "reflow_process_view.h"
#include "types/display.h"
#include "controllers/reflow_controller.h"

void ReflowProcessView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

void ReflowProcessView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = ReflowController::getInstance();
    const auto& step = controller.getModel().getCurrentStep();

    float temp = controller.getCurrentTemp();
    float target = controller.getCurrentTargetTemp();
    float elapsed = controller.getElapsedMsInStep() / 1000.0f;

    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, step.label.c_str(), STYLE_BOLD);
    canvas.setColor(GREEN);
    canvas.printFixed(0, 14, ("Target: " + std::to_string((int)target) + "C").c_str(), STYLE_NORMAL);
    canvas.setColor(BLUE);
    canvas.printFixed(0, 24, ("Current: " + std::to_string((int)temp) + "C").c_str(), STYLE_NORMAL);
    canvas.setColor(WHITE);
    canvas.printFixed(0, 34, ("Elapsed: " + std::to_string((int)elapsed) + "s").c_str(), STYLE_NORMAL);

    // Optional: progress bar
    int barWidth = (int)((elapsed / (step.durationMs / 1000.0f)) * 96);
    if (barWidth > 96) barWidth = 96;
    canvas.setColor(GREY);
    canvas.drawRect(0, 54, 96, 8);
    canvas.setColor(GREEN);
    canvas.drawRect(0, 54, barWidth, 8);

    display.drawCanvas(0, 0, canvas);
}

void ReflowProcessView::handleEncoderLongPress() {
    ReflowController::getInstance().cancel();
}

void ReflowProcessView::handleEncoderPress() {
    // Optional: no-op
}

void ReflowProcessView::handleEncoderUp() {
    // Optional: no-op
}

void ReflowProcessView::handleEncoderDown() {
    // Optional: no-op
}

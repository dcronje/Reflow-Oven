#include "reflow_precheck_view.h"
#include "types/display.h"
#include "controllers/reflow_controller.h"

void ReflowPrecheckView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

void ReflowPrecheckView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = ReflowController::getInstance();
    const auto& curve = controller.getModel().getActiveCurve();
    float currentTemp = controller.getCurrentTemp();

    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, ("Curve: " + curve.name).c_str(), STYLE_NORMAL);
    canvas.printFixed(0, 12, ("Oven: " + std::to_string((int)currentTemp) + "C").c_str(), STYLE_NORMAL);
    canvas.printFixed(0, 24, ("Start when < " + std::to_string((int)curve.minimumStartTempC) + "C").c_str(), STYLE_NORMAL);

    if (currentTemp <= curve.minimumStartTempC) {
        readyToStart = true;
        canvas.setColor(GREEN);
        canvas.printFixed(0, 40, "Insert PCB +", STYLE_BOLD);
        canvas.printFixed(0, 50, "Press to start", STYLE_BOLD);
    } else {
        readyToStart = false;
        canvas.setColor(RED);
        canvas.printFixed(0, 40, "Cooling...", STYLE_BOLD);
    }

    display.drawCanvas(0, 0, canvas);
}

void ReflowPrecheckView::handleEncoderPress() {
    if (readyToStart) {
        ReflowController::getInstance().confirmStart();
    }
}

void ReflowPrecheckView::handleEncoderLongPress() {
    ReflowController::getInstance().cancel();
}

void ReflowPrecheckView::handleEncoderUp() {
    // Optional: no-op
}

void ReflowPrecheckView::handleEncoderDown() {
    // Optional: no-op
}

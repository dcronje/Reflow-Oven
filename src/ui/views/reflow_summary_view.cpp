#include "reflow_summary_view.h"
#include "types/display.h"
#include "controllers/reflow_controller.h"

void ReflowSummaryView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
}

void ReflowSummaryView::render(DisplaySSD1331_96x64x8_SPI& display) {
    canvas.clear();

    auto& controller = ReflowController::getInstance();
    const auto& curve = controller.getModel().getActiveCurve();

    canvas.setColor(WHITE);
    canvas.printFixed(0, 0, "Reflow Complete!", STYLE_BOLD);
    canvas.setColor(GREEN);
    canvas.printFixed(0, 14, ("Curve: " + curve.name).c_str(), STYLE_NORMAL);

    // Optionally: more step stats later
    canvas.setColor(WHITE);
    canvas.printFixed(0, 40, "Press to return", STYLE_NORMAL);

    display.drawCanvas(0, 0, canvas);
}

void ReflowSummaryView::handleEncoderPress() {
    // Return to main menu
    UIViewService::getInstance().showView("main-menu");
}

void ReflowSummaryView::handleEncoderLongPress() {
    // Return to main menu
    UIViewService::getInstance().showView("main-menu");
}

void ReflowSummaryView::handleEncoderUp() {
    // No action
}

void ReflowSummaryView::handleEncoderDown() {
    // No action
}

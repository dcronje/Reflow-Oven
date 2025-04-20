#include "reflow_curve_selection_view.h"
#include "controllers/reflow_controller.h"
#include "library/reflow_curve_library.h"

void ReflowCurveSelectionView::init() {
    canvas.setFixedFont(ssd1306xled_font6x8);
    menu.setTitle("Select Curve");

    const auto& curves = ReflowCurveLibrary::getBuiltInCurves();

    for (const auto& curve : curves) {
        menu.addItem(MenuItemElement(curve.name, [curve]() {
            ReflowController::getInstance().getModel().setActiveCurve(curve);
            ReflowController::getInstance().requestStart();
        }));
    }
}

void ReflowCurveSelectionView::render(DisplaySSD1331_96x64x8_SPI& display) {
    menu.render(canvas);
    display.drawCanvas(0, 0, canvas);
}

void ReflowCurveSelectionView::handleEncoderUp() {
    menu.moveUp();
}

void ReflowCurveSelectionView::handleEncoderDown() {
    menu.moveDown();
}

void ReflowCurveSelectionView::handleEncoderPress() {
    menu.select();
}

void ReflowCurveSelectionView::handleEncoderLongPress() {
    UIViewService::getInstance().showView("main-menu");
}

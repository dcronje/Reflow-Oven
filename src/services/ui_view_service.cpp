#include "ui_view_service.h"
#include "constants.h"

UIViewService& UIViewService::getInstance() {
    static UIViewService instance;
    return instance;
}

UIViewService::UIViewService()
    : display(DISPLAY_SPI_RST_GPIO, {
        .cs = {DISPLAY_SPI_CS_GPIO},
        .dc = DISPLAY_SPI_DC_GPIO,
        .frequency = DISPLAY_SPI_BAUDRATE,
        .scl = DISPLAY_SPI_CLK_GPIO,
        .sda = DISPLAY_SPI_MOSI_GPIO
      })
{}

void UIViewService::init() {
    display.begin();
    display.clear();

    xTaskCreate(renderTaskEntry, "UIViewTask", 1024, this, tskIDLE_PRIORITY + 1, &renderTask);
}

void UIViewService::renderTaskEntry(void* param) {
    reinterpret_cast<UIViewService*>(param)->renderLoop();
}

void UIViewService::renderLoop() {
    while (true) {
        if (currentView) {
            currentView->render(display);
        }
        vTaskDelay(pdMS_TO_TICKS(33)); // ~30 FPS
    }
}

void UIViewService::registerView(const std::string& name, std::unique_ptr<UIView> view) {
    view->init();
    views[name] = std::move(view);
}

void UIViewService::showView(const std::string& name) {
    auto it = views.find(name);
    if (it != views.end()) {
        currentViewName = name;
        currentView = it->second.get();
    }
}

void UIViewService::handleEncoderUp() {
    if (currentView) currentView->handleEncoderUp();
}

void UIViewService::handleEncoderDown() {
    if (currentView) currentView->handleEncoderDown();
}

void UIViewService::handleEncoderPress() {
    if (currentView) currentView->handleEncoderPress();
}

void UIViewService::handleEncoderLongPress() {
    if (currentView) currentView->handleEncoderLongPress();
}

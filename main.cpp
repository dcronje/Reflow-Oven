#include <cstdio>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "hardware/gpio.h"
#include "isr_handlers.h"
#include "services/ui_view_service.h"
#include "services/electronics_cooling_service.h"
#include "services/temperature_control_service.h"
#include "services/door_service.h"
#include "services/sensor_service.h"
#include "services/interaction_service.h"
#include "services/calibration_service.h"
#include "services/buzzer_service.h"
#include "services/communication_service.h"
#include "controllers/main_menu_controller.h"
#include "controllers/reflow_controller.h"
#include "controllers/calibration_controller.h"

#define WATCHDOG_TIMEOUT_MS 5000 // Watchdog timeout in milliseconds

// Task priorities
#define SYSTEM_TASK_PRIORITY       2
#define UI_TASK_PRIORITY           3
#define CONTROL_TASK_PRIORITY      4
#define WATCHDOG_TASK_PRIORITY     5

// Task stack sizes
#define SYSTEM_TASK_STACK_SIZE     2048
#define UI_TASK_STACK_SIZE         4096
#define CONTROL_TASK_STACK_SIZE    2048
#define WATCHDOG_TASK_STACK_SIZE   512

// Task handles
TaskHandle_t uiTaskHandle = NULL;
TaskHandle_t controlTaskHandle = NULL;
TaskHandle_t watchdogTaskHandle = NULL;

extern "C"
{
    extern "C" void vApplicationIdleHook(void) {
        printf("Free heap: %u bytes\n", xPortGetFreeHeapSize());
    }

    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
    {
        printf("Stack Overflow in Task: %s\n", pcTaskName);
        while (true)
        {
            tight_loop_contents();
        }
    }

    void vApplicationMallocFailedHook(void)
    {
        printf("Malloc Failed!\n");
        while (true)
        {
            tight_loop_contents();
        }
    }

    void Default_Handler(void)
    {
        uint32_t irq_num;
        asm volatile("mrs %0, ipsr" : "=r"(irq_num));
        printf("Unhandled IRQ: %ld (Core %d)\n", irq_num & 0x1FF, get_core_num()); // Extract IRQ number
        while (1)
            ;
    }
}

// UI task - will run on core 0
void uiTask(void* params) {
    stdio_init_all(); // 🔁 Re-init on core 0
    printf("UI Task started on core %d\n", get_core_num());
    gpio_set_irq_callback(&sharedISR);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    // Initialize UI-related services
    UIViewService& uiService = UIViewService::getInstance();
    uiService.init();
    
    // Create and initialize RootView
    auto rootView = std::make_unique<RootView>();
    rootView->init(uiService.getDisplay());
    
    // Register RootView as the encoder event handler
    uiService.registerEncoderEventHandler(rootView.get());
    
    // Initialize communication service before other services that depend on it
    CommunicationService::getInstance().init();
    
    InteractionService::getInstance().init();
    BuzzerService::getInstance().init();
    
    // UI services are initialized and running in their own tasks
    // No need to do anything else in this task
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Just yield to other tasks
    }
}

// Control task - will run on core 1
void controlTask(void* params) {
    printf("Control Task started on core %d\n", get_core_num());
    
    // Initialize hardware control services
    DoorService::getInstance().init();
    SensorService::getInstance().init();
    CalibrationService::getInstance().init();
    ElectronicsCoolingService::getInstance().init();
    TemperatureControlService::getInstance().init();
    
    // Main control loop
    while (true) {
        // Process control logic - call methods that actually exist in the services
        // Each service implements its own internal task/thread for updates
        
        // Just run the FreeRTOS scheduler - actual work is done in service tasks
        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms update rate
    }
}

// Watchdog task - can run on either core
void watchdogTask(void* params) {
    printf("Watchdog Task started on core %d\n", get_core_num());
    
    while (true) {
        watchdog_update();
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS / 2));
    }
}

int main()
{
    stdio_init_all();
    // gpio_set_irq_callback(&sharedISR);
    printf("Starting Reflow Oven System on RP2350 (dual-core)...\n");

    // Initialize the watchdog with a timeout
    watchdog_enable(WATCHDOG_TIMEOUT_MS, 1);

    // // Create the UI task (core affinity set to core 0)
    BaseType_t result = xTaskCreate(uiTask, "UITask", UI_TASK_STACK_SIZE, 
                                   nullptr, UI_TASK_PRIORITY, &uiTaskHandle);
    configASSERT(result == pdPASS);
    
    // Set UI task to run only on core 0
    UBaseType_t uiCoreAffinityMask = (1 << 0);
    vTaskCoreAffinitySet(uiTaskHandle, uiCoreAffinityMask);
    
    // Create the control task (core affinity set to core 1)
    result = xTaskCreate(controlTask, "ControlTask", CONTROL_TASK_STACK_SIZE, 
                        nullptr, CONTROL_TASK_PRIORITY, &controlTaskHandle);
    configASSERT(result == pdPASS);
    
    // Set control task to run only on core 1
    UBaseType_t controlCoreAffinityMask = (1 << 1);
    vTaskCoreAffinitySet(controlTaskHandle, controlCoreAffinityMask);
    
    // Create the watchdog task (can run on either core)
    result = xTaskCreate(watchdogTask, "WatchdogTask", WATCHDOG_TASK_STACK_SIZE, 
                        nullptr, WATCHDOG_TASK_PRIORITY, &watchdogTaskHandle);
    configASSERT(result == pdPASS);
    
    // Let the watchdog run on any available core
    UBaseType_t watchdogCoreAffinityMask = (1 << 0) | (1 << 1);
    vTaskCoreAffinitySet(watchdogTaskHandle, watchdogCoreAffinityMask);
    
    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (true) {
        // If we do reach here, something went wrong
        printf("ERROR: Scheduler exited!\n");
        sleep_ms(1000);
    }
    
    return 0;
}

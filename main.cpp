#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
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
#include "controllers/main_menu_controller.h"
#include "controllers/reflow_controller.h"
#include "controllers/calibration_controller.h"

#define WATCHDOG_TIMEOUT_MS 5000 // Watchdog timeout in milliseconds

// Task priorities
#define SYSTEM_TASK_PRIORITY 2
#define WATCHDOG_TASK_PRIORITY 3

// Task stack sizes
#define SYSTEM_TASK_STACK_SIZE 2048
#define WATCHDOG_TASK_STACK_SIZE 256

extern "C"
{
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
        printf("Unhandled IRQ: %ld\n", irq_num & 0x1FF); // Extract IRQ number
        while (1)
            ;
    }
}

// Watchdog task
void watchdogKickTask(void* params) {
    while (true) {
        watchdog_update();
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_TIMEOUT_MS / 2));
    }
}

int main()
{
    stdio_init_all();
    gpio_set_irq_callback(&sharedISR);
    printf("Starting Reflow Oven System...\n");

    // Initialize the watchdog with a timeout, this will reset the system if not regularly kicked
    watchdog_enable(WATCHDOG_TIMEOUT_MS, 1);

    // Initialize the system services
    DoorService::getInstance().init();
    SensorService::getInstance().init();
    UIViewService::getInstance().init();
    InteractionService::getInstance().init();
    CalibrationService::getInstance().init();
    ElectronicsCoolingService::getInstance().init();
    TemperatureControlService::getInstance().init();
    BuzzerService::getInstance().init();

    // Initialize the controllers
    ReflowController::getInstance().init();
    MainMenuController::getInstance().init();
    CalibrationController::getInstance().init();

    // Create the watchdog task
    xTaskCreate(watchdogKickTask, "WatchdogTask", WATCHDOG_TASK_STACK_SIZE, nullptr, WATCHDOG_TASK_PRIORITY, nullptr);
    
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

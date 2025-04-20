#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/gpio.h"
#include "isr_handlers.h"
#include "services/ui_view_service.h"
#include "controllers/main_menu_controller.h"

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

void initViews()
{
    auto& ui = UIViewService::getInstance();
    MainMenuController::getInstance().registerViews(ui);
    // ReflowController::getInstance().registerViews(ui);
}

int main()
{
    stdio_init_all();
    gpio_set_irq_callback(&sharedISR);
    printf("Starting Reflow Oven System...\n");

    // Initialize the watchdog with a timeout, this will reset the system if not regularly kicked
    watchdog_enable(WATCHDOG_TIMEOUT_MS, 1);
    initViews();
    // create tasks here

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

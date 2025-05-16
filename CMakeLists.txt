# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Use pico2 board type - SDK will handle the memory settings for RP2350
set(PICO_BOARD pico2 CACHE STRING "Board type")
set(PICO_PLATFORM rp2350)

# Define PICO_RP2350 for FreeRTOS config
add_compile_definitions(PICO_RP2350=1)

# For FreeRTOS, ensure we're building for the Cortex-M33
add_compile_options(-mcpu=cortex-m33 -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=softfp)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# Initialize the project
project(Reflow-Oven C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# FreeRTOS configuration
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS/FreeRTOS-Kernel CACHE STRING "")
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE STRING "")

# Important: Make sure to use the FreeRTOS_Kernel_import.cmake from the repo
include(FreeRTOS_Kernel_import.cmake)

# MPU wrapper configuration for compatibility
add_compile_definitions(configUSE_MPU_WRAPPERS_V1=1)

# Define SVC number for the scheduler
add_compile_definitions(portSVC_START_SCHEDULER=102)

# Ensure multicore support is enabled for dual-core operation
add_compile_definitions(LIB_PICO_MULTICORE=1)

set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/src/lv_conf.h")
add_definitions(-DLV_CONF_PATH=\"${CMAKE_SOURCE_DIR}/src/lv_conf.h\")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/onewire)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/nanopb)

file(REMOVE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl/src/draw/sw/blend/helium/lv_blend_helium.S
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl/src/draw/sw/blend/neon/lv_blend_neon.S
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS/FreeRTOS-Kernel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS/FreeRTOS-Kernel/portable/ThirdParty/Community-Supported-Ports/GCC/RP2350_ARM_NTZ/non_secure
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/lvgl)

target_include_directories(lvgl PUBLIC
    ${PICO_SDK_PATH}/src/common/pico_base/include
    ${PICO_SDK_PATH}/src/common/pico_base_headers/include
    ${CMAKE_BINARY_DIR}/generated/pico_base
)

target_link_libraries(lvgl PUBLIC
    pico_base_headers
    pico_base
    hardware_sync
    pico_runtime
)

# Add executable. Default name is the project name, version 0.1

file(GLOB SRC_FILES
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/services/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/models/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/library/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/controllers/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/ui/*.cpp"
)

add_executable(Reflow-Oven main.cpp ${SRC_FILES})

pico_set_program_name(Reflow-Oven "Reflow-Oven")
pico_set_program_version(Reflow-Oven "0.1")

pico_generate_pio_header(Reflow-Oven ${CMAKE_CURRENT_LIST_DIR}/servo.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Reflow-Oven 1)
pico_enable_stdio_usb(Reflow-Oven 0)

# Add the standard library to the build
target_link_libraries(Reflow-Oven
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Reflow-Oven PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/types
        ${CMAKE_CURRENT_LIST_DIR}/lib/nanopb
        ${CMAKE_CURRENT_LIST_DIR}/lib/lvgl
        # RTOS includes are handled by the FreeRTOS_Kernel_import.cmake
)

# Add any user requested libraries
target_link_libraries(Reflow-Oven 
        hardware_spi
        hardware_i2c
        hardware_pio
        hardware_pwm
        hardware_flash
        hardware_adc
        pico_multicore           # Explicitly link multicore library for dual-core operation
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_one_wire
        lvgl
        )

pico_add_extra_outputs(Reflow-Oven)


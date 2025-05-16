#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"

const int COOLING_FAN_PWM_GPIO = 2; // GPIO pin for the cooling fan
const int LIGHTS_GPIO = 3; // GPIO pin for the cooling fan tachometer
const int HEATER_SSR_GPIO = 4; // GPIO pin for the element SSR
const int SSR_TEMP_GPIO = 5; // GPIO pin for the SSR temperature sensor
const int BUZZER_GPIO = 6; // GPIO pin for the buzzer
const int ENCODER_DC_GPIO = 7; // GPIO pin for the encoder data
const int ENCODER_CLK_GPIO = 8; // GPIO pin for the encoder clock
const int ENCODER_SW_GPIO = 9; // GPIO pin for the encoder switch
const int DISPLAY_BACKLIGHT_GPIO = 10; // GPIO pin for the display backlight
const int DISPLAY_SPI_RST_GPIO = 11;  // Reset (RST) – GPIO 13
const int DISPLAY_SPI_DC_GPIO = 12;   // Data/Command (DC) – GPIO 14
const int DISPLAY_SPI_CS_GPIO = 13;   // Chip Select (CS) – GPIO 17
const int DISPLAY_SPI_CLK_GPIO = 14;  // SPI clock (SCK) – GPIO 18
const int DISPLAY_SPI_MOSI_GPIO = 15; // SPI data (MOSI) – GPIO 19
const int THERMOCOUPLE_SPI_MISO_GPIO = 16; // SPI MISO – GPIO 12
const int THERMOCOUPLE_CS_GPIO = 17; // GPIO pin for the thermocouple
const int THERMOCOUPLE_SPI_CLK_GPIO = 18; // SPI clock (SCK) – GPIO 10
const int DOOR_OPEN_SWITCH_GPIO = 19; // Door open limit switch
const int AMBIENT_TEMP_I2C_SDA_GPIO = 20; // I2C data (SDA) – GPIO 20
const int AMBIENT_TEMP_I2C_SCL_GPIO = 21; // I2C clock (SCL) – GPIO 21
const int DOOR_CLOSED_SWITCH_GPIO = 22; // Door closed limit switch
const int DOOR_SERVO_FEEDBACK_GPIO = 26; // Servo feedback
const int DOOR_SERVO_CONTROL_GPIO = 27; // Servo PWM signal
const int SERVO_POWER_GPIO = 28; // MOSFET control for servo power


// Interaction constants
const int DEBOUNCE_TIME_MS = 50;     // Button debounce time in milliseconds
const int LONG_PRESS_TIME_MS = 500;   // Long press detection time in milliseconds  
const int QUEUE_WAIT_TIME_MS = 10;    // Queue wait time in milliseconds


// SPI configurations
// OPTION 1: Use SPI0 with existing pins (recommended)
#define DISPLAY_SPI_PORT spi1  // Use SPI0 since pins 14-15 are used

// OPTION 2: Use SPI1 but change pins (alternative)
// #define DISPLAY_SPI_PORT spi1  // SPI1 typically uses pins 10-13
// const int DISPLAY_SPI_CLK_GPIO = 10;  // SPI1 clock (SCK)
// const int DISPLAY_SPI_MOSI_GPIO = 11; // SPI1 data (MOSI)
#define DISPLAY_SPI_BAUDRATE 1000000 // 1 MHz
#define THERMOCOUPLE_SPI_PORT spi1
#define THERMOCOUPLE_SPI_BAUDRATE 1000000 // 1 MHz

// I2C configurations for ambient temperature sensor
#define AMBIENT_TEMP_I2C_PORT i2c0
#define SHT30_I2C_ADDR 0x44
#define AMBIENT_TEMP_I2C_BAUDRATE 400000 // 400 kHz

// Control constants
#define MIN_COOLING_CHANGE_INTERVAL 250
#define HEATER_CONTROL_PERIOD_MS 250  // 250ms time-proportional control window
#define TEMPERATURE_CONTROL_KP 1.0f   // Proportional control constant

// PID control constants
#define REFLOW_PID_PROPORTIONAL_GAIN 2.0f
#define REFLOW_PID_INTEGRAL_GAIN 0.1f
#define REFLOW_PID_DERIVATIVE_GAIN 0.5f

// Settings constants
#define SETTINGS_MAGIC 0xDEADBEEF
#define FLASH_TARGET_OFFSET 0x100000
#define CALIBRATION_FLASH_OFFSET 0x100000  // Adjust based on your flash layout

// Display Configuration
#define DISPLAY_SPI_FREQ 20000000  // 40MHz
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define DISPLAY_X_OFFSET 0
#define DISPLAY_Y_OFFSET 0
#define DISPLAY_ROTATION ST7789_TFT::TFT_Degrees_90  // Rotate 90 degrees for landscape orientation
#define DISPLAY_BACKLIGHT_PWM_WRAP 65535



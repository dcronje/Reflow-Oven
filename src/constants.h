#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"

// Display SPI pins
const int DISPLAY_SPI_CLK_GPIO = 18;  // SPI clock (SCK) – GPIO 18
const int DISPLAY_SPI_MOSI_GPIO = 19; // SPI data (MOSI) – GPIO 19
const int DISPLAY_SPI_CS_GPIO = 17;   // Chip Select (CS) – GPIO 17
const int DISPLAY_SPI_DC_GPIO = 20;   // Data/Command (DC) – GPIO 14
const int DISPLAY_SPI_RST_GPIO = 21;  // Reset (RST) – GPIO 13
const int DISPLAY_BACKLIGHT_GPIO = 22;

// Encoder pins
const int ENCODER_CLK_GPIO = 2;
const int ENCODER_DC_GPIO = 3;
const int ENCODER_SW_GPIO = 4;

// Interaction constants
const int DEBOUNCE_TIME_MS = 50;     // Button debounce time in milliseconds
const int LONG_PRESS_TIME_MS = 500;   // Long press detection time in milliseconds  
const int QUEUE_WAIT_TIME_MS = 10;    // Queue wait time in milliseconds

// Buzzer pin
const int BUZZER_GPIO = 8; // GPIO pin for the buzzer

// Thermocouple pins
const int THERMOCOUPLE_CS_GPIO = 1; // GPIO pin for the thermocouple
const int THERMOCOUPLE_SPI_CLK_GPIO = 10; // SPI clock (SCK) – GPIO 10
const int THERMOCOUPLE_SPI_MISO_GPIO = 12; // SPI MISO – GPIO 12

// Heater control pins (SSR controlled)
const int HEATER_SSR_GPIO = 19; // GPIO pin for the element SSR

const int FAN_CONTROL_GPIO = 5; // GPIO pin for the fan control
const int FAN_TACH_GPIO = 7;    // GPIO pin for the fan tachometer

const int TOP_LEFT_VENT_SERVO_GPIO = 6; // GPIO pin for the top left vent servo
const int TOP_RIGHT_VENT_SERVO_GPIO = 7; // GPIO pin for the top right vent servo
const int BOTTOM_LEFT_VENT_SERVO_GPIO = 8; // GPIO pin for the bottom left vent servo
const int BOTTOM_RIGHT_VENT_SERVO_GPIO = 9; // GPIO pin for the bottom right vent servo

const int MIN_COOLING_CHANGE_INTERVAL = 250;

const uint8_t SHT30_I2C_ADDR = 0x44;

// Safety pins
const int SSR_TEMP_GPIO = 9; // GPIO pin for the SSR temperature sensor
const int COOLING_FAN_PWM_GPIO = 10; // GPIO pin for the cooling fan
const int COOLING_FAN_TACH_GPIO = 10; // GPIO pin for the cooling fan

// Door control
const int SERVO_POWER_GPIO = 2;      // MOSFET control for servo power
const int DOOR_OPEN_SWITCH_GPIO = 3; // Door open limit switch
const int DOOR_CLOSED_SWITCH_GPIO = 4; // Door closed limit switch
const int DOOR_SERVO_CONTROL_GPIO = 5;       // Servo PWM signal
const int DOOR_SERVO_FEEDBACK_GPIO = 5;

const int BUZZER_PIN  = 16;  // Adjust this to match your actual buzzer pin


// SPI configurations
#define DISPLAY_SPI_PORT spi0
#define DISPLAY_SPI_BAUDRATE 1000000 // 1 MHz
#define THERMOCOUPLE_SPI_PORT spi1
#define THERMOCOUPLE_SPI_BAUDRATE 1000000 // 1 MHz

// I2C configurations for ambient temperature sensor
#define AMBIENT_TEMP_I2C_PORT i2c0
#define AMBIENT_TEMP_I2C_SDA_GPIO 8
#define AMBIENT_TEMP_I2C_SCL_GPIO 9
#define AMBIENT_TEMP_I2C_BAUDRATE 400000 // 400 kHz

// Control constants
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
#define DISPLAY_SPI_FREQ 40000000  // 40MHz
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320
#define DISPLAY_X_OFFSET 0
#define DISPLAY_Y_OFFSET 0
#define DISPLAY_ROTATION ST7789_TFT::TFT_Degrees_90  // Rotate 90 degrees for landscape orientation
#define DISPLAY_BACKLIGHT_PWM_WRAP 65535



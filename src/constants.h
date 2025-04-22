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

// Encoder pins
const int ENCODER_CLK_GPIO = 2;
const int ENCODER_DC_GPIO = 3;
const int ENCODER_SW_GPIO = 4;

// Buzzer pin
const int BUZZER_GPIO = 8; // GPIO pin for the buzzer

// Thermocouple pins
const int THERMOCOUPLE_BACK_CS_GPIO = 0; // GPIO pin for the thermocouple backlight
const int THERMOCOUPLE_FRONT_CS_GPIO = 1; // GPIO pin for the thermocouple frontlight
const int THERMOCOUPLE_SPI_CLK_GPIO = 10; // SPI clock (SCK) – GPIO 10
const int THERMOCOUPLE_SPI_MISO_GPIO = 12; // SPI MISO – GPIO 12

// Heater control pins (SSR controlled)
const int HEATER_FRONT_SSR_GPIO = 19; // GPIO pin for the front element SSR
const int HEATER_BACK_SSR_GPIO = 20; // GPIO pin for the back element SSR

// Ventilation pins
const int VENT_RIGHT_SERVO_GPIO = 4; // GPIO pin for the right vent servo
const int VENT_LEFT_SERVO_GPIO = 4; // GPIO pin for the left vent servo
const int FAN_SERVO_GPIO = 5; // GPIO pin for the fan servo
const int FAN_CONTROL_GPIO = 5; // GPIO pin for the fan control
const int FAN_TACH_GPIO = 7;    // GPIO pin for the fan tachometer

const int TOP_LEFT_VENT_SERVO_GPIO = 6; // GPIO pin for the top left vent servo
const int TOP_RIGHT_VENT_SERVO_GPIO = 7; // GPIO pin for the top right vent servo
const int BOTTOM_LEFT_VENT_SERVO_GPIO = 8; // GPIO pin for the bottom left vent servo
const int BOTTOM_RIGHT_VENT_SERVO_GPIO = 9; // GPIO pin for the bottom right vent servo

const int MIN_COOLING_CHANGE_INTERVAL = 250;

const uint8_t SHT30_I2C_ADDR = 0x44;

// Safety pins
const int DOOR_SWITCH_GPIO = 8;  // Door safety switch

#define SERVO_MIN_PULSE 1950  // ~1 ms
#define SERVO_MAX_PULSE 3900  // ~2 ms
#define SERVO_PERIOD_TICKS 39062  // 20 ms period

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

// Door control
#define SERVO_POWER_GPIO 2      // MOSFET control for servo power
#define DOOR_OPEN_SWITCH_GPIO 3 // Door open limit switch
#define DOOR_CLOSED_SWITCH_GPIO 4 // Door closed limit switch
#define DOOR_SERVO_GPIO 5       // Servo PWM signal
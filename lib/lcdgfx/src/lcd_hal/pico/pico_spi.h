/*
    MIT License

    Copyright (c) 2018-2022, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/*
 * @file hal/pico/pico_spi.h LCDGFX Raspberry Pi Pico Interface communication functions
 */

#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"

#if defined(PICO_USE_SPI1)
#define PICO_SPI spi1
#else
#define PICO_SPI spi0
#endif

/**
 * Class implements SPI support for Arduino platforms
 */
class PicoSpi
{
public:
    /**
     * Creates instance of spi implementation for Arduino platform.
     * @param csPin chip select pin to use, -1 if not required
     * @param dcPin data command pin to use
     * @param clkPin clk pin to use, -1 to use default. Doesn't work on all controllers
     * @param mosiPin mosi pin to use, -1 to use default. Doesn't work on all controllers
     * @param freq frequency in HZ to run spi bus at
     */
    PicoSpi(SPlatformSpiConfig &config);
    ~PicoSpi();

    /**
     * Initializes spi interface
     */
    void begin();

    /**
     * Closes spi interface
     */
    void end();

    /**
     * Starts communication with SSD1306 display.
     */
    void start();

    /**
     * Ends communication with SSD1306 display.
     */
    void stop();

    /**
     * Sends byte to SSD1306 device
     * @param data - byte to send
     */
    void send(uint8_t data);

    /**
     * @brief Sends bytes to SSD1306 device
     *
     * Sends bytes to SSD1306 device. This functions gives
     * ~ 30% performance increase than ssd1306_intf.send.
     *
     * @param buffer - bytes to send
     * @param size - number of bytes to send
     */
    void sendBuffer(const uint8_t *buffer, uint16_t size);

private:
    int8_t m_cs;
    int8_t m_dc;
    int8_t m_clk;
    int8_t m_mosi;
    uint32_t m_frequency;
};

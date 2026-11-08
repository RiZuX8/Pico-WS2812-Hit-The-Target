/**
 * @file WS2812.hpp
 * @brief This library stores the LED colors in a buffer and sends them to a WS2812 strip on the Pico W using PIO timing.
 * @author RiZuX8
 */
#ifndef WS2812_HPP
#define WS2812_HPP

#include "Arduino.h"

const uint8_t ledsAmount = 24;

#pragma pack(push, 1)

/** @brief Single color channel value. */
struct Color {
    uint8_t value;
};

/** @brief One LED in GRB order. */
struct Led {
    Color green;
    Color red;
    Color blue;
};

/** @brief LED storage for the whole strip. */
struct LedStripBuffer {
    Led leds[ledsAmount];
};

#pragma pack(pop)

/**
 * @brief Control a WS2812 LED strip.
 * @details
 * Call setPin() first, then begin(), then update LED values with setLed() and transmit them with send().
 */
class WS2812 {
    public:
        bool setPin(uint8_t pin);
        bool begin();
        bool setLed(int position, uint8_t red, uint8_t green, uint8_t blue);
        bool send();

    private:
        LedStripBuffer ledBytes{};
        uint8_t dataPin = 255; // To check if the pin was set correctly
        uint sm = 0;
        uint offset = 0;
};

#endif

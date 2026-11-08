/**
 * @file WS2812.cpp
 * @brief Functions for the WS2812 LED strip library.
 * @author RiZuX8
 */

#include "WS2812.hpp"
#include "hardware/clocks.h"
#include "hardware/pio.h"

/** @brief Number of clock cycles for the initial low/high segment. */
static const uint8_t t1 = 2;

/** @brief Number of clock cycles for the middle segment. */
static const uint8_t t2 = 5;

/** @brief Number of clock cycles for the final segment. */
static const uint8_t t3 = 3;

/** @brief Target bit frequency for WS2812 data stream (Hz). */
static const uint32_t freq = 800000; // 800 Kbps

/** @brief Create the PIO instance */
static const PIO pio = pio0;

/** @brief PIO program instructions */
static const uint16_t prog[] = {
    0x6221,
    0x1123,
    0x1400,
    0xa442,
};

/** @brief Load the PIO program */
static const struct pio_program program = {
    .instructions = prog,
    .length = 4,
    .origin = -1,
};

/**
 * @brief Configure a PIO state machine for WS2812 output.
 *
 * This function builds the `pio_sm_config` required to run the WS2812 PIO program.
 * @param pio     The PIO instance.
 * @param sm      The state machine index within the PIO instance.
 * @param offset  The program offset returned by `pio_add_program`.
 * @param pin     The GPIO pin number connected to the LED strip data line.
 * @param hz      The target bit frequency for the WS2812 waveform (Hz).
 */
void setup_pio(PIO pio, uint8_t sm, uint8_t offset, uint8_t pin, uint32_t hz) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + 0, offset + 3);     // Loop over the 4 PIO instructions
    sm_config_set_sideset(&c, 1, false, false);         // 1 sideset bit for precise timing
    sm_config_set_sideset_pins(&c, pin);                // Connect sideset to the chosen GPIO
    sm_config_set_out_shift(&c, false, true, 24);       // Shift out 24 bits, MSB first (GRB)

    float div = clock_get_hz(clk_sys) / static_cast<float>(hz * (t1 + t2 + t3)); // Calculate the clock signal for the requested frequency
    sm_config_set_clkdiv(&c, div); // Set timing

    pio_gpio_init(pio, pin);                            // Claim the GPIO for PIO control
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true); // Set the pin as output for this state machine
    pio_sm_init(pio, sm, offset, &c);                   // Load config and start at the program offset
    pio_sm_set_enabled(pio, sm, true);                  // Enable the state machine
}

/**
 * @brief Store an RGB value for a single LED in the buffer.
 * 
 * @param position Index of the LED in the buffer.
 * @param red      8 bit red channel value.
 * @param green    8 bit green channel value.
 * @param blue     8 bit blue channel value.
 * @return true on success, false if `position` is out of range.
 */
bool WS2812::setLed(int position, uint8_t red, uint8_t green, uint8_t blue) {
    if (position < 0 || position >= ledsAmount) {
        return false;
    }
    Led led;
    led.red.value = red;
    led.green.value = green;
    led.blue.value = blue;
    ledBytes.leds[position] = led;
    return true;
}

/**
 * @brief Select the GPIO pin used for the WS2812 data line.
 * 
 * Call this before `begin()`;
 *
 * @param pin GPIO pin number.
 * @return true when the pin was stored.
 */
bool WS2812::setPin(uint8_t pin) {
    dataPin = pin;
    return true;
}

/**
 * @brief Initialize the PIO program and state machine for the configured pin.
 * 
 * Call this after `setPin()`. The pin must be set before calling this function.
 * 
 * @return true on success, false if the pin was not set.
 */
bool WS2812::begin() {
    if (dataPin == 255) {
        return false;
    }
    offset = pio_add_program(pio, &program);
    setup_pio(pio, sm, offset, dataPin, freq);
    return true;
}

/**
 * @brief Send the full LED buffer to the strip.
 * 
 * The buffer is sent in GRB order, MSB first.
 *
 * @return true on successful transmission.
 */
bool WS2812::send() {
    for (size_t i = 0; i < ledsAmount; ++i) {
        const Led& led = ledBytes.leds[i];
        uint32_t grb = (led.green.value << 16)
                 | (led.red.value << 8)
                 | led.blue.value;
        pio_sm_put_blocking(pio, sm, grb << 8);
    }
    return true;
}

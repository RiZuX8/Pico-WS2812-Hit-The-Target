# Pico WS2812 Hit The Target

Hit The Target is a reaction timing game for the Raspberry Pi Pico / Pico W using a 24-LED WS2812 ring and a button, driven by a custom lightweight PIO (Programmable I/O) WS2812 library.

## Hit The Target (Game)

A target is randomly placed on the 24-LED ring. An active runner LED moves around the ring continuously. The objective is to press the button when the runner hits the target zone.

- **Level 0**: Target width of 3 LEDs. 5 speed steps (50 ms down to 20 ms).
- **Level 1**: Target width of 2 LEDs. 6 speed steps (70 ms down to 20 ms).
- **Level 2**: Target width of 1 LED. 9 speed steps (80 ms down to 5 ms).
- **Hit**: The strip flashes blue and advances the speed step or level.
- **Miss**: The strip flashes red/green and resets score and target.
- **Win**: Clearing all 3 levels triggers a win flash animation.

## Wiring

| Component | Pico Pin | Description |
|---|---|---|
| WS2812 DI | GP0 | LED data line |
| Pushbutton | GP1 | Connected to GND (uses internal `INPUT_PULLUP`) |
| Power | VBUS (5V) | 5V supply for the WS2812 strip |
| Ground | GND | Common ground between Pico and WS2812 |

## WS2812 Library

The included WS2812 library uses the Pico's PIO (Programmable I/O) state machine to achieve the precise timing required by WS2812 LED strips at 800 Kbps without relying on external libraries.

### Public functions

- `setPin(pin)`: select the GPIO pin that drives the WS2812 data line.
- `begin()`: load and start the PIO program after a pin has been selected.
- `setLed(position, red, green, blue)`: store one RGB color in the internal buffer.
- `send()`: transmit the full buffer to the LED strip.

### Usage

1. Create a `WS2812` object.
2. Call `setPin(...)` with the GPIO pin that is connected to the strip.
3. Call `begin()` once before sending data.
4. Update LEDs with `setLed(...)` for each position.
5. Call `send()` to transmit the buffer to the strip.

### Design

- Data is transmitted at 800 Kbps. A `0` bit is represented by a short HIGH then long LOW. A `1` bit is a long HIGH then short LOW. This requires very accurate timing.
- On the Pico W a compact PIO program is used with a few instructions and three timing constants (`t1`, `t2`, `t3`) to compose one WS2812 bit cycle. With a 125 ns PIO cycle this produces the required 1.25 µs bit period.
- The library uses a LED buffer and sends colors in GRB order as expected by the WS2812 module.

### Notes

- The configuration uses a fixed buffer size of 24 LEDs.
- Colors are sent in WS2812 GRB order.

### SPI attempt

- An earlier attempt used SPI by overclocking the line and encoding WS2812 bits. On the Pico this proved less reliable than the PIO approach because timing is very strict. PIO gives stable timing and is therefore preferred.

### Sources

- Raspberry Pi Pico PIO WS2812 example: https://github.com/raspberrypi/pico-examples/tree/master/pio/ws2812
- Pico SDK hardware documentation: https://www.raspberrypi.com/documentation/pico-sdk/hardware.html
- WS2812 datasheet: https://cdn-shop.adafruit.com/datasheets/WS2812.pdf

## License

This project is licensed under the [MIT License](LICENSE).

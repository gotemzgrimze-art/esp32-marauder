# Hardware Notes

This repo defaults to a generic `esp32dev` board and a serial interface so it can
boot without assuming a specific handheld PCB.

## Minimal Bring-Up

- ESP32 development board
- USB power/data connection
- Serial monitor at `115200`

## Optional Inputs

Edit [`include/board_config.h`](../include/board_config.h) to map:

- `PIN_BATTERY_ADC` for battery monitoring
- `PIN_BUTTON_UP`
- `PIN_BUTTON_SELECT`
- `PIN_BUTTON_DOWN`
- `PIN_STATUS_LED`

## Battery Measurement

If your LiPo voltage is measured through a resistor divider:

1. Connect the divided battery voltage to an ADC-capable pin.
2. Set `PIN_BATTERY_ADC` to that pin number.
3. Set `BATTERY_DIVIDER_RATIO` to match the divider.

Example:

- 100k / 100k divider halves the battery voltage
- use `BATTERY_DIVIDER_RATIO = 2.0f`

## Future Display Support

This starter uses serial output first because display modules vary heavily between
handheld builds. The clean next step is to add one of:

- SSD1306 over I2C
- ST7735/ST7789 over SPI
- a small web UI in AP mode for configuration only

If you want, the repo can be extended from this base toward your exact screen,
buttons, battery circuit, and enclosure layout.

## Why This Structure

Well-known Marauder hardware variants commonly add:

- persistent settings
- battery status integration
- onboard storage
- multiple board-specific hardware mappings

This repo adopts the safe subset of those ideas while keeping the firmware
generic and passive-first.

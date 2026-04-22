# ESP32 Marauder Handheld

This repository is now a real starter project instead of an empty placeholder.
It targets a generic ESP32 handheld build and intentionally focuses on safe,
passive functionality:

- passive Wi-Fi scanning
- passive BLE discovery
- device telemetry and battery hooks
- persistent settings stored in NVS
- scan logging to onboard SPIFFS storage
- serial command interface with settings and log management
- hardware configuration points for a custom handheld

It does **not** implement deauthentication, packet injection, credential capture,
beacon spam, or similar offensive features.

## Project Layout

```text
.
├── .github/workflows/build.yml
├── docs/COMPARISON.md
├── docs/HARDWARE.md
├── include/
│   ├── app_config.h
│   └── board_config.h
├── src/main.cpp
└── platformio.ini
```

## Requirements

- [PlatformIO](https://platformio.org/)
- an ESP32 board supported by the Arduino framework

The default environment is `esp32dev`.

## CI

GitHub Actions now runs `platformio run` on pushes to `main` and on pull
requests. That gives the repository a basic compile check even when you are not
building locally.

## Flashing

```bash
cd /home/gotemzgrimze/projects/esp32-marauder
platformio run
platformio run --target upload
platformio device monitor
```

## Serial Commands

Open the serial monitor at `115200` baud, then use:

- `help`
- `status`
- `settings`
- `wifi`
- `ble`
- `show wifi`
- `show ble`
- `auto on`
- `auto off`
- `set wifi_interval <seconds>`
- `set ble_interval <seconds>`
- `set logging on|off`
- `set led on|off`
- `logs`
- `erase logs`
- `save`
- `clear`

## Storage And Settings

Inspired by the better-supported Marauder builds, this repo now keeps state
between boots:

- settings are persisted in ESP32 NVS
- Wi-Fi scan summaries are appended to `/wifi_log.csv`
- BLE scan summaries are appended to `/ble_log.csv`

This keeps the project useful as a handheld field scanner without inheriting the
offensive feature set those projects also contain.

## Configuration

Start with these files:

- [`include/app_config.h`](include/app_config.h) for scan timing and limits
- [`include/board_config.h`](include/board_config.h) for pins and battery scaling

## What To Do Next

The repo is now at the point where it can be adapted to your exact hardware
instead of starting from nothing. The highest-value next steps are:

1. wire in your real screen and buttons
2. replace the serial menu with a handheld UI layer
3. swap SPIFFS logs for SD card storage if your board has a slot
4. tune scan intervals for power consumption

## Notes

Because this environment does not currently have PlatformIO or Arduino CLI
installed, the scaffold was created carefully but not compiled locally here.

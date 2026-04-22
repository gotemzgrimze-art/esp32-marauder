#pragma once

constexpr char APP_NAME[] = "ESP32 Marauder Handheld";
constexpr char APP_VERSION[] = "0.2.0";

// Default scan timings. Tweak these for your battery budget.
constexpr unsigned long APP_BOOT_DELAY_MS = 1500;
constexpr unsigned long APP_WIFI_SCAN_INTERVAL_MS = 30000;
constexpr unsigned long APP_BLE_SCAN_INTERVAL_MS = 45000;
constexpr unsigned long APP_MIN_SCAN_INTERVAL_MS = 5000;
constexpr unsigned long APP_MAX_SCAN_INTERVAL_MS = 300000;
constexpr uint32_t APP_SERIAL_BAUD = 115200;

constexpr uint8_t APP_MAX_WIFI_RESULTS = 16;
constexpr uint8_t APP_MAX_BLE_RESULTS = 12;
constexpr uint8_t APP_BLE_SCAN_SECONDS = 4;

constexpr bool APP_DEFAULT_AUTO_SCAN = true;
constexpr bool APP_DEFAULT_LOGGING = true;
constexpr bool APP_DEFAULT_LED_ENABLED = true;

constexpr char APP_SETTINGS_NAMESPACE[] = "marauder";
constexpr char APP_WIFI_LOG_PATH[] = "/wifi_log.csv";
constexpr char APP_BLE_LOG_PATH[] = "/ble_log.csv";

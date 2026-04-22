#pragma once

#include <Arduino.h>

#include "app_config.h"

struct WiFiEntry {
  String ssid;
  int32_t rssi;
  uint8_t channel;
  String encryption;
};

struct BleEntry {
  String address;
  String name;
  int rssi;
};

struct AppSettings {
  bool autoScanEnabled = APP_DEFAULT_AUTO_SCAN;
  bool loggingEnabled = APP_DEFAULT_LOGGING;
  bool ledEnabled = APP_DEFAULT_LED_ENABLED;
  unsigned long wifiScanIntervalMs = APP_WIFI_SCAN_INTERVAL_MS;
  unsigned long bleScanIntervalMs = APP_BLE_SCAN_INTERVAL_MS;
};

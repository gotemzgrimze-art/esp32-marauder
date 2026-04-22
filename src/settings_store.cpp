#include "settings_store.h"

#include <Preferences.h>

#include "app_config.h"

namespace settings_store {
namespace {

Preferences preferences;

unsigned long clampInterval(unsigned long value, unsigned long fallback) {
  if (value < APP_MIN_SCAN_INTERVAL_MS || value > APP_MAX_SCAN_INTERVAL_MS) {
    return fallback;
  }
  return value;
}

}  // namespace

void begin() {
  preferences.begin(APP_SETTINGS_NAMESPACE, false);
}

AppSettings load() {
  AppSettings settings;
  settings.autoScanEnabled = preferences.getBool("auto_scan", APP_DEFAULT_AUTO_SCAN);
  settings.loggingEnabled = preferences.getBool("logging", APP_DEFAULT_LOGGING);
  settings.ledEnabled = preferences.getBool("led", APP_DEFAULT_LED_ENABLED);
  settings.wifiScanIntervalMs = clampInterval(
      preferences.getULong("wifi_int", APP_WIFI_SCAN_INTERVAL_MS), APP_WIFI_SCAN_INTERVAL_MS);
  settings.bleScanIntervalMs = clampInterval(
      preferences.getULong("ble_int", APP_BLE_SCAN_INTERVAL_MS), APP_BLE_SCAN_INTERVAL_MS);
  return settings;
}

bool save(const AppSettings &settings) {
  const bool okAuto = preferences.putBool("auto_scan", settings.autoScanEnabled) > 0;
  const bool okLogging = preferences.putBool("logging", settings.loggingEnabled) > 0;
  const bool okLed = preferences.putBool("led", settings.ledEnabled) > 0;
  const bool okWifi = preferences.putULong("wifi_int", settings.wifiScanIntervalMs) > 0;
  const bool okBle = preferences.putULong("ble_int", settings.bleScanIntervalMs) > 0;
  return okAuto && okLogging && okLed && okWifi && okBle;
}

}  // namespace settings_store

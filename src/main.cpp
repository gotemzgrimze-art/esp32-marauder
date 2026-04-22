#include <Arduino.h>
#include <BLEDevice.h>
#include <WiFi.h>

#include "app_config.h"
#include "app_state.h"
#include "board_config.h"
#include "log_store.h"
#include "settings_store.h"

namespace {

WiFiEntry wifiEntries[APP_MAX_WIFI_RESULTS];
BleEntry bleEntries[APP_MAX_BLE_RESULTS];

AppSettings settings;
size_t wifiEntryCount = 0;
size_t bleEntryCount = 0;
unsigned long lastWifiScanMs = 0;
unsigned long lastBleScanMs = 0;
bool bleReady = false;
bool logStorageReady = false;
String commandBuffer;

String authModeToString(wifi_auth_mode_t mode) {
  switch (mode) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-ENT";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3-PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2/WPA3";
    default:
      return "Unknown";
  }
}

float readBatteryVoltage() {
  if (PIN_BATTERY_ADC < 0) {
    return NAN;
  }

  const uint16_t raw = analogRead(PIN_BATTERY_ADC);
  return (static_cast<float>(raw) / BATTERY_ADC_MAX) * BATTERY_ADC_REFERENCE_V *
         BATTERY_DIVIDER_RATIO;
}

void printPrompt() {
  Serial.print("\n> ");
}

void printDivider() {
  Serial.println(F("--------------------------------------------------"));
}

void printBanner() {
  printDivider();
  Serial.printf("%s v%s\n", APP_NAME, APP_VERSION);
  Serial.println(F("Safe build: passive Wi-Fi/BLE scanning, telemetry, settings, logs"));
  printDivider();
}

void printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  help        Show this menu"));
  Serial.println(F("  status      Show system status"));
  Serial.println(F("  settings    Show persisted settings"));
  Serial.println(F("  wifi        Run a passive Wi-Fi scan"));
  Serial.println(F("  ble         Run a passive BLE scan"));
  Serial.println(F("  show wifi   Show the last Wi-Fi scan results"));
  Serial.println(F("  show ble    Show the last BLE scan results"));
  Serial.println(F("  auto on     Enable periodic background scans"));
  Serial.println(F("  auto off    Disable periodic background scans"));
  Serial.println(F("  set wifi_interval <seconds>"));
  Serial.println(F("  set ble_interval <seconds>"));
  Serial.println(F("  set logging on|off"));
  Serial.println(F("  set led on|off"));
  Serial.println(F("  logs        Show log storage status"));
  Serial.println(F("  erase logs  Clear stored scan logs"));
  Serial.println(F("  save        Persist current settings to NVS"));
  Serial.println(F("  clear       Clear the serial console"));
}

void printSettings() {
  printDivider();
  Serial.println(F("Settings"));
  Serial.printf("Auto scan: %s\n", settings.autoScanEnabled ? "enabled" : "disabled");
  Serial.printf("Wi-Fi interval: %lu seconds\n", settings.wifiScanIntervalMs / 1000);
  Serial.printf("BLE interval: %lu seconds\n", settings.bleScanIntervalMs / 1000);
  Serial.printf("Logging: %s\n", settings.loggingEnabled ? "enabled" : "disabled");
  Serial.printf("LED: %s\n", settings.ledEnabled ? "enabled" : "disabled");
  printDivider();
}

void printStatus() {
  const float batteryVoltage = readBatteryVoltage();
  const uint64_t mac = ESP.getEfuseMac();

  printDivider();
  Serial.printf("App: %s v%s\n", APP_NAME, APP_VERSION);
  Serial.printf("Chip ID: %04X%08X\n",
                static_cast<uint16_t>(mac >> 32),
                static_cast<uint32_t>(mac));
  Serial.printf("Uptime: %lu ms\n", millis());
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Auto scan: %s\n", settings.autoScanEnabled ? "enabled" : "disabled");
  Serial.printf("Logging: %s\n", settings.loggingEnabled ? "enabled" : "disabled");
  Serial.printf("LED: %s\n", settings.ledEnabled ? "enabled" : "disabled");
  Serial.printf("Last Wi-Fi scan: %lu ms\n", lastWifiScanMs);
  Serial.printf("Last BLE scan: %lu ms\n", lastBleScanMs);
  Serial.printf("Saved Wi-Fi results: %u\n", static_cast<unsigned>(wifiEntryCount));
  Serial.printf("Saved BLE results: %u\n", static_cast<unsigned>(bleEntryCount));
  Serial.printf("Storage: %s\n", logStorageReady ? "SPIFFS ready" : "SPIFFS unavailable");

  if (isnan(batteryVoltage)) {
    Serial.println(F("Battery: disabled (set PIN_BATTERY_ADC in include/board_config.h)"));
  } else {
    Serial.printf("Battery: %.2f V\n", batteryVoltage);
  }

  printDivider();
}

void printWifiResults() {
  if (wifiEntryCount == 0) {
    Serial.println(F("[Wi-Fi] No cached scan results."));
    return;
  }

  Serial.printf("[Wi-Fi] Last cached results: %u\n", static_cast<unsigned>(wifiEntryCount));
  for (size_t i = 0; i < wifiEntryCount; ++i) {
    const char *ssid = wifiEntries[i].ssid.isEmpty() ? "<hidden>" : wifiEntries[i].ssid.c_str();
    Serial.printf("  %02u. RSSI %4d dBm | CH %2u | %-9s | %s\n",
                  static_cast<unsigned>(i + 1), wifiEntries[i].rssi,
                  wifiEntries[i].channel, wifiEntries[i].encryption.c_str(), ssid);
  }
}

void runWifiScan() {
  Serial.println(F("\n[Wi-Fi] Starting passive scan..."));
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);

  const int networkCount = WiFi.scanNetworks(false, true);
  wifiEntryCount = 0;

  if (networkCount <= 0) {
    Serial.println(F("[Wi-Fi] No networks found."));
    lastWifiScanMs = millis();
    return;
  }

  const size_t stored = min(static_cast<size_t>(networkCount),
                            static_cast<size_t>(APP_MAX_WIFI_RESULTS));
  wifiEntryCount = stored;

  for (size_t i = 0; i < stored; ++i) {
    wifiEntries[i].ssid = WiFi.SSID(i);
    wifiEntries[i].rssi = WiFi.RSSI(i);
    wifiEntries[i].channel = WiFi.channel(i);
    wifiEntries[i].encryption =
        authModeToString(static_cast<wifi_auth_mode_t>(WiFi.encryptionType(i)));
  }

  lastWifiScanMs = millis();

  Serial.printf("[Wi-Fi] Found %d networks, showing %u\n", networkCount,
                static_cast<unsigned>(wifiEntryCount));
  printWifiResults();

  if (settings.loggingEnabled && logStorageReady) {
    if (log_store::appendWifiScan(lastWifiScanMs, wifiEntries, wifiEntryCount)) {
      Serial.println(F("[Wi-Fi] Results appended to SPIFFS log."));
    } else {
      Serial.println(F("[Wi-Fi] Failed to write scan log."));
    }
  }

  WiFi.scanDelete();
}

void ensureBleReady() {
  if (bleReady) {
    return;
  }

  BLEDevice::init("ESP32-Marauder-Handheld");
  bleReady = true;
}

void printBleResults() {
  if (bleEntryCount == 0) {
    Serial.println(F("[BLE] No cached scan results."));
    return;
  }

  Serial.printf("[BLE] Last cached results: %u\n", static_cast<unsigned>(bleEntryCount));
  for (size_t i = 0; i < bleEntryCount; ++i) {
    Serial.printf("  %02u. RSSI %4d dBm | %s | %s\n",
                  static_cast<unsigned>(i + 1), bleEntries[i].rssi,
                  bleEntries[i].address.c_str(), bleEntries[i].name.c_str());
  }
}

void runBleScan() {
  Serial.println(F("\n[BLE] Starting passive scan..."));
  ensureBleReady();

  BLEScan *scanner = BLEDevice::getScan();
  scanner->setActiveScan(false);
  scanner->setInterval(160);
  scanner->setWindow(80);

  BLEScanResults results = scanner->start(APP_BLE_SCAN_SECONDS, false);
  const int count = results.getCount();
  bleEntryCount = min(static_cast<size_t>(count),
                      static_cast<size_t>(APP_MAX_BLE_RESULTS));

  if (count <= 0) {
    Serial.println(F("[BLE] No devices found."));
    lastBleScanMs = millis();
    scanner->clearResults();
    return;
  }

  for (size_t i = 0; i < bleEntryCount; ++i) {
    BLEAdvertisedDevice device = results.getDevice(i);
    bleEntries[i].address = device.getAddress().toString().c_str();
    bleEntries[i].name = device.haveName() ? device.getName().c_str() : String("<unnamed>");
    bleEntries[i].rssi = device.getRSSI();
  }

  lastBleScanMs = millis();

  Serial.printf("[BLE] Found %d devices, showing %u\n", count,
                static_cast<unsigned>(bleEntryCount));
  printBleResults();

  if (settings.loggingEnabled && logStorageReady) {
    if (log_store::appendBleScan(lastBleScanMs, bleEntries, bleEntryCount)) {
      Serial.println(F("[BLE] Results appended to SPIFFS log."));
    } else {
      Serial.println(F("[BLE] Failed to write scan log."));
    }
  }

  scanner->clearResults();
}

void clearConsole() {
  Serial.print(F("\033[2J\033[H"));
}

bool saveSettings() {
  const bool ok = settings_store::save(settings);
  Serial.println(ok ? F("Settings saved.") : F("Failed to save settings."));
  return ok;
}

bool parseOnOffValue(const String &value, bool &out) {
  if (value == "on") {
    out = true;
    return true;
  }

  if (value == "off") {
    out = false;
    return true;
  }

  return false;
}

void printLogSummary() {
  printDivider();
  Serial.println(log_store::summary());
  printDivider();
}

void handleSetCommand(const String &command) {
  const String prefix = "set ";
  if (!command.startsWith(prefix)) {
    Serial.println(F("Malformed set command."));
    return;
  }

  String rest = command.substring(prefix.length());
  rest.trim();
  const int split = rest.indexOf(' ');
  if (split < 0) {
    Serial.println(F("Usage: set <name> <value>"));
    return;
  }

  String key = rest.substring(0, split);
  String value = rest.substring(split + 1);
  key.trim();
  value.trim();

  if (key == "wifi_interval" || key == "ble_interval") {
    const long seconds = value.toInt();
    if (seconds <= 0) {
      Serial.println(F("Interval must be a positive number of seconds."));
      return;
    }

    const unsigned long intervalMs = static_cast<unsigned long>(seconds) * 1000UL;
    if (intervalMs < APP_MIN_SCAN_INTERVAL_MS || intervalMs > APP_MAX_SCAN_INTERVAL_MS) {
      Serial.printf("Interval must be between %lu and %lu seconds.\n",
                    APP_MIN_SCAN_INTERVAL_MS / 1000,
                    APP_MAX_SCAN_INTERVAL_MS / 1000);
      return;
    }

    if (key == "wifi_interval") {
      settings.wifiScanIntervalMs = intervalMs;
    } else {
      settings.bleScanIntervalMs = intervalMs;
    }

    saveSettings();
    printSettings();
    return;
  }

  bool parsed = false;
  bool on = false;
  if (key == "logging") {
    parsed = parseOnOffValue(value, on);
    if (!parsed) {
      Serial.println(F("Use `set logging on` or `set logging off`."));
      return;
    }
    settings.loggingEnabled = on;
    saveSettings();
    printSettings();
    return;
  }

  if (key == "led") {
    parsed = parseOnOffValue(value, on);
    if (!parsed) {
      Serial.println(F("Use `set led on` or `set led off`."));
      return;
    }
    settings.ledEnabled = on;
    saveSettings();
    printSettings();
    return;
  }

  Serial.printf("Unknown setting: %s\n", key.c_str());
}

void handleCommand(String command) {
  command.trim();
  command.toLowerCase();

  if (command.isEmpty()) {
    printPrompt();
    return;
  }

  if (command == "help") {
    printHelp();
  } else if (command == "status") {
    printStatus();
  } else if (command == "settings") {
    printSettings();
  } else if (command == "wifi") {
    runWifiScan();
  } else if (command == "ble") {
    runBleScan();
  } else if (command == "show wifi") {
    printWifiResults();
  } else if (command == "show ble") {
    printBleResults();
  } else if (command == "auto on") {
    settings.autoScanEnabled = true;
    saveSettings();
    Serial.println(F("Background scanning enabled and persisted."));
  } else if (command == "auto off") {
    settings.autoScanEnabled = false;
    saveSettings();
    Serial.println(F("Background scanning disabled and persisted."));
  } else if (command.startsWith("set ")) {
    handleSetCommand(command);
  } else if (command == "logs") {
    printLogSummary();
  } else if (command == "erase logs") {
    if (log_store::clearLogs()) {
      Serial.println(F("Logs cleared."));
    } else {
      Serial.println(F("Failed to clear logs."));
    }
  } else if (command == "save") {
    saveSettings();
  } else if (command == "clear") {
    clearConsole();
    printBanner();
    printHelp();
  } else {
    Serial.printf("Unknown command: %s\n", command.c_str());
    Serial.println(F("Type `help` for the available commands."));
  }

  printPrompt();
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    const char incoming = static_cast<char>(Serial.read());

    if (incoming == '\r') {
      continue;
    }

    if (incoming == '\n') {
      handleCommand(commandBuffer);
      commandBuffer = "";
      continue;
    }

    commandBuffer += incoming;
  }
}

void updateHeartbeat() {
  if (PIN_STATUS_LED < 0 || !settings.ledEnabled) {
    if (PIN_STATUS_LED >= 0) {
      digitalWrite(PIN_STATUS_LED, LOW);
    }
    return;
  }

  const bool ledOn = (millis() / 500) % 2 == 0;
  digitalWrite(PIN_STATUS_LED, ledOn ? HIGH : LOW);
}

}  // namespace

void setup() {
  Serial.begin(APP_SERIAL_BAUD);
  delay(APP_BOOT_DELAY_MS);

  if (PIN_STATUS_LED >= 0) {
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);
  }

  if (PIN_BATTERY_ADC >= 0) {
    analogReadResolution(12);
  }

  settings_store::begin();
  settings = settings_store::load();
  logStorageReady = log_store::begin();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);

  printBanner();
  Serial.println(logStorageReady ? F("SPIFFS logging ready.") : F("SPIFFS logging unavailable."));
  printHelp();
  printSettings();
  printPrompt();
}

void loop() {
  readSerialCommands();
  updateHeartbeat();

  if (settings.autoScanEnabled) {
    const unsigned long now = millis();

    if (lastWifiScanMs == 0 || now - lastWifiScanMs >= settings.wifiScanIntervalMs) {
      runWifiScan();
      printPrompt();
    }

    if (lastBleScanMs == 0 || now - lastBleScanMs >= settings.bleScanIntervalMs) {
      runBleScan();
      printPrompt();
    }
  }

  delay(20);
}

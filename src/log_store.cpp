#include "log_store.h"

#include <FS.h>
#include <SPIFFS.h>

#include "app_config.h"

namespace log_store {
namespace {

bool mounted = false;

String csvEscape(String value) {
  value.replace("\"", "\"\"");
  value.replace("\n", " ");
  value.replace("\r", " ");
  return "\"" + value + "\"";
}

bool ensureFileWithHeader(const char *path, const char *header) {
  if (SPIFFS.exists(path)) {
    return true;
  }

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }

  file.println(header);
  file.close();
  return true;
}

size_t fileSizeOrZero(const char *path) {
  if (!SPIFFS.exists(path)) {
    return 0;
  }

  File file = SPIFFS.open(path, FILE_READ);
  if (!file) {
    return 0;
  }

  const size_t size = file.size();
  file.close();
  return size;
}

}  // namespace

bool begin() {
  mounted = SPIFFS.begin(true);
  if (!mounted) {
    return false;
  }

  const bool wifiReady = ensureFileWithHeader(APP_WIFI_LOG_PATH, "timestamp_ms,ssid,rssi,channel,encryption");
  const bool bleReady = ensureFileWithHeader(APP_BLE_LOG_PATH, "timestamp_ms,address,name,rssi");
  return wifiReady && bleReady;
}

bool isReady() {
  return mounted;
}

bool appendWifiScan(unsigned long timestampMs, const WiFiEntry *entries, size_t count) {
  if (!mounted || !ensureFileWithHeader(APP_WIFI_LOG_PATH, "timestamp_ms,ssid,rssi,channel,encryption")) {
    return false;
  }

  File file = SPIFFS.open(APP_WIFI_LOG_PATH, FILE_APPEND);
  if (!file) {
    return false;
  }

  for (size_t i = 0; i < count; ++i) {
    file.printf("%lu,%s,%ld,%u,%s\n",
                timestampMs,
                csvEscape(entries[i].ssid).c_str(),
                static_cast<long>(entries[i].rssi),
                entries[i].channel,
                csvEscape(entries[i].encryption).c_str());
  }

  file.close();
  return true;
}

bool appendBleScan(unsigned long timestampMs, const BleEntry *entries, size_t count) {
  if (!mounted || !ensureFileWithHeader(APP_BLE_LOG_PATH, "timestamp_ms,address,name,rssi")) {
    return false;
  }

  File file = SPIFFS.open(APP_BLE_LOG_PATH, FILE_APPEND);
  if (!file) {
    return false;
  }

  for (size_t i = 0; i < count; ++i) {
    file.printf("%lu,%s,%s,%d\n",
                timestampMs,
                csvEscape(entries[i].address).c_str(),
                csvEscape(entries[i].name).c_str(),
                entries[i].rssi);
  }

  file.close();
  return true;
}

bool clearLogs() {
  if (!mounted) {
    return false;
  }

  if (SPIFFS.exists(APP_WIFI_LOG_PATH)) {
    SPIFFS.remove(APP_WIFI_LOG_PATH);
  }

  if (SPIFFS.exists(APP_BLE_LOG_PATH)) {
    SPIFFS.remove(APP_BLE_LOG_PATH);
  }

  const bool wifiReady = ensureFileWithHeader(APP_WIFI_LOG_PATH, "timestamp_ms,ssid,rssi,channel,encryption");
  const bool bleReady = ensureFileWithHeader(APP_BLE_LOG_PATH, "timestamp_ms,address,name,rssi");
  return wifiReady && bleReady;
}

String summary() {
  if (!mounted) {
    return "Log storage unavailable";
  }

  String out;
  out += "Log storage: SPIFFS ready\n";
  out += "  Wi-Fi log: ";
  out += APP_WIFI_LOG_PATH;
  out += " (";
  out += String(fileSizeOrZero(APP_WIFI_LOG_PATH));
  out += " bytes)\n";
  out += "  BLE log: ";
  out += APP_BLE_LOG_PATH;
  out += " (";
  out += String(fileSizeOrZero(APP_BLE_LOG_PATH));
  out += " bytes)";
  return out;
}

}  // namespace log_store

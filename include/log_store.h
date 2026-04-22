#pragma once

#include <Arduino.h>

#include "app_state.h"

namespace log_store {

bool begin();
bool isReady();
bool appendWifiScan(unsigned long timestampMs, const WiFiEntry *entries, size_t count);
bool appendBleScan(unsigned long timestampMs, const BleEntry *entries, size_t count);
bool clearLogs();
String summary();

}  // namespace log_store

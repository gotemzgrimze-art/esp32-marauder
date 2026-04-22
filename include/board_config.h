#pragma once

// Set optional hardware pins here. Leave as -1 when unused.
constexpr int PIN_BATTERY_ADC = -1;
constexpr int PIN_BUTTON_UP = -1;
constexpr int PIN_BUTTON_SELECT = -1;
constexpr int PIN_BUTTON_DOWN = -1;

// Battery measurement settings for a resistor-divider input.
constexpr float BATTERY_ADC_REFERENCE_V = 3.30f;
constexpr float BATTERY_DIVIDER_RATIO = 2.0f;
constexpr uint16_t BATTERY_ADC_MAX = 4095;

// If you have an onboard LED, set the pin here for heartbeat/status use.
constexpr int PIN_STATUS_LED = 2;

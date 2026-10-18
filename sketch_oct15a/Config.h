#pragma once

#include <Arduino.h>

namespace Config {

// =====================================================
// ESP32-C3
// =====================================================
constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;

// =====================================================
// LCD
// =====================================================
constexpr uint8_t MAX_LCD_COLS = 20;
constexpr uint8_t MAX_LCD_ROWS = 4;
constexpr uint8_t MAX_DISPLAYS = 4;
constexpr uint8_t MAX_PAGES_PER_DISPLAY = 8;
constexpr unsigned long LCD_CHECK_INTERVAL = 1000;

// =====================================================
// DS18B20
// =====================================================
constexpr uint8_t TEMP_PIN = 2;
constexpr uint8_t TEMP_SENSOR_COUNT = 2;
constexpr uint8_t TEMP_RESOLUTION_BITS = 10;
constexpr uint32_t TEMP_CONVERSION_TIME_MS = 188;

constexpr uint8_t TEMP_SUPPLY_ROM[8] = {
    0x28, 0xFF, 0x64, 0x1E, 0xFD, 0xA6, 0xB4, 0xE1
};

constexpr uint8_t TEMP_RETURN_ROM[8] = {
    0x28, 0xFF, 0x64, 0x1E, 0xFD, 0xD3, 0x88, 0xB5
};

// =====================================================
// ALARM CONFIGURATION
// =====================================================
constexpr uint8_t MAX_ACTIVE_ALARMS = 16;
constexpr unsigned long ALARM_SCREEN_TIME = 2250UL;
constexpr uint8_t MAX_ALARM_CHANNELS = 40;

// =====================================================
// TEMPERATURE ALARM SETTINGS
// =====================================================
constexpr float TEMP_HYSTERESIS = 0.5f;

// Supply temperature
constexpr float POD_TEMP_NORMAL = 25.0f;
constexpr float POD_TEMP_CRITICAL_LOW = 15.0f;
constexpr float POD_TEMP_WARN_LOW = 20.0f;
constexpr float POD_TEMP_WARN_HIGH = 30.0f;
constexpr float POD_TEMP_CRITICAL_HIGH = 32.0f;
constexpr float POD_TEMP_RETRIGGER_STEP = 1.0f;

// Return temperature
constexpr float OBR_TEMP_NORMAL = 25.0f;
constexpr float OBR_TEMP_CRITICAL_LOW = 15.0f;
constexpr float OBR_TEMP_WARN_LOW = 20.0f;
constexpr float OBR_TEMP_WARN_HIGH = 30.0f;
constexpr float OBR_TEMP_CRITICAL_HIGH = 32.0f;
constexpr float OBR_TEMP_RETRIGGER_STEP = 1.0f;

// =====================================================
// PRESSURE ALARM SETTINGS
// All pressure values are in atmospheres.
// These are temporary example values.
// =====================================================
constexpr float PRESSURE_HYSTERESIS = 0.05f;

// City water pressure
constexpr float CITY_PRESS_NORMAL = 2.0f;
constexpr float CITY_PRESS_CRITICAL_LOW = 0.5f;
constexpr float CITY_PRESS_WARN_LOW = 1.0f;
constexpr float CITY_PRESS_WARN_HIGH = 8.0f;
constexpr float CITY_PRESS_CRITICAL_HIGH = 10.0f;
constexpr float CITY_PRESS_RETRIGGER_STEP = 0.1f;

// Supply pressure
constexpr float POD_PRESS_NORMAL = 3.5f;
constexpr float POD_PRESS_CRITICAL_LOW = 2.5f;
constexpr float POD_PRESS_WARN_LOW = 3.0f;
constexpr float POD_PRESS_WARN_HIGH = 4.0f;
constexpr float POD_PRESS_CRITICAL_HIGH = 4.5f;
constexpr float POD_PRESS_RETRIGGER_STEP = 0.1f;

// Return pressure
constexpr float OBR_PRESS_NORMAL = 2.0f;
constexpr float OBR_PRESS_CRITICAL_LOW = 1.0f;
constexpr float OBR_PRESS_WARN_LOW = 1.5f;
constexpr float OBR_PRESS_WARN_HIGH = 2.5f;
constexpr float OBR_PRESS_CRITICAL_HIGH = 3.0f;
constexpr float OBR_PRESS_RETRIGGER_STEP = 0.1f;

// =====================================================
// BUZZER AND BUTTON
// =====================================================
constexpr uint8_t BUZZER_PIN = 126;
constexpr uint8_t BUTTON_PIN = 128;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;

constexpr uint8_t TEMP_LED_PIN = 20 /* GPIO светодиода температуры */;
constexpr uint8_t PRESSURE_LED_PIN = 21 /* GPIO светодиода давления */;
constexpr uint32_t LED_BLINK_INTERVAL_MS = 500;

}
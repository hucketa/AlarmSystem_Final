#pragma once
#include <Arduino.h>
namespace Config {
    constexpr uint8_t SDA_PIN = 8;
    constexpr uint8_t SCL_PIN = 9;
    constexpr uint8_t MAX_LCD_COLS = 20;
    constexpr uint8_t MAX_LCD_ROWS = 4;
    constexpr uint8_t MAX_DISPLAYS = 4;
    constexpr uint8_t MAX_PAGES_PER_DISPLAY = 8;
    constexpr unsigned long LCD_CHECK_INTERVAL = 1000;
    constexpr uint8_t TEMP_PIN = 2;
    constexpr uint8_t TEMP_SENSOR_COUNT = 2;
    constexpr uint8_t TEMP_RESOLUTION_BITS = 10;
    constexpr uint32_t TEMP_CONVERSION_TIME_MS = 188;
    constexpr uint8_t TEMP_SUPPLY_ROM[8] = {0x28, 0xFF, 0x64, 0x1E, 0xFD, 0xA6, 0xB4, 0xE1};
    constexpr uint8_t TEMP_RETURN_ROM[8] = {0x28, 0xFF, 0x64, 0x1E, 0xFD, 0xD3, 0x88, 0xB5};
    // =====================================================
    // ALARM CONFIGURATION
    // =====================================================
    constexpr uint8_t MAX_ACTIVE_ALARMS = 16;
    constexpr unsigned long ALARM_SCREEN_TIME = 2250UL;
    // =====================================================
    // TEMPERATURE ALARM SETTINGS
    // =====================================================
    constexpr float TEMP_HYSTERESIS = 0.5f;
    constexpr float POD_TEMP_WARN_LOW = 20.0f;
    constexpr float POD_TEMP_CRITICAL_LOW = 15.0f;
    constexpr float POD_TEMP_WARN_HIGH = 30.0f;
    constexpr float POD_TEMP_CRITICAL_HIGH = 32.0f;
    constexpr float OBR_TEMP_WARN_LOW = 20.0f;
    constexpr float OBR_TEMP_CRITICAL_LOW = 15.0f;
    constexpr float OBR_TEMP_WARN_HIGH = 30.0f;
    constexpr float OBR_TEMP_CRITICAL_HIGH = 32.0f;
}
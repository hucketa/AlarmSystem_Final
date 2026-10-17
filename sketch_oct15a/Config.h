
#pragma once

#include <Arduino.h>

namespace Config {
    // =====================================================
    // I2C
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
    constexpr unsigned long DEFAULT_PAGE_INTERVAL = 3000;
    // =====================================================
    // TEMPERATURE SENSORS
    // =====================================================
    constexpr uint8_t TEMP_PIN = 2;
    constexpr uint8_t TEMP_SENSOR_COUNT = 2;
    constexpr uint32_t TEMP_CONVERSION_TIME_MS = 750;
    constexpr uint8_t TEMP_SUPPLY_ROM[8] = {
        0x28, 0xFF, 0x64, 0x1E, 0xFD, 0xA6, 0xB4, 0xE1
    };
    constexpr uint8_t TEMP_RETURN_ROM[8] = {
        0x28, 0xFF, 0x64, 0x1E, 0xFD, 0xD3, 0x88, 0xB5
    };
}

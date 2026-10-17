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
    constexpr unsigned long DEFAULT_PAGE_INTERVAL = 3000;
}

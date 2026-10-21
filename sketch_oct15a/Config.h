#pragma once

#include <Arduino.h>

namespace Config {

// =====================================================
// LCD DISPLAYS
// =====================================================
constexpr uint8_t TEMP_LCD_ADDRESS = 0x25;
constexpr uint8_t PRESS_LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;

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
constexpr uint8_t BUZZER_PIN = 20;
constexpr uint8_t BUTTON_PIN = 21;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;

constexpr uint8_t TEMP_LED_PIN = 120 /* GPIO светодиода температуры */;
constexpr uint8_t PRESSURE_LED_PIN = 126 /* GPIO светодиода давления */;
constexpr uint32_t LED_BLINK_INTERVAL_MS = 500;


// =====================================================
// PRESSURE SENSOR CONFIGURATION
// =====================================================
constexpr uint8_t PRESSURE_SUPPLY_PIN = 0;
constexpr uint8_t PRESSURE_SAMPLES = 32;
constexpr uint32_t PRESSURE_SAMPLE_INTERVAL_US = 500;
constexpr bool PRESSURE_CALIBRATION_MODE = false;

struct PressureCalibrationPoint {
    uint16_t raw;
    float pressure;
};

const PressureCalibrationPoint SUPPLY_PRESSURE_TABLE[] = {
    {333,  0.0f},
    {473,  0.5f},
    {585,  1.0f},
    {735,  1.5f},
    {860,  2.0f},
    {990,  2.5f},
    {1120, 3.0f},
    {1250, 3.5f},
    {1385, 4.0f},
    {1520, 4.5f},
    {1660, 5.0f},
    {1800, 5.5f},
    {1945, 6.0f}
};

constexpr size_t SUPPLY_PRESSURE_TABLE_SIZE =
    sizeof(SUPPLY_PRESSURE_TABLE) / sizeof(SUPPLY_PRESSURE_TABLE[0]);

constexpr uint8_t PRESSURE_RETURN_PIN = 1;

const Config::PressureCalibrationPoint RETURN_PRESSURE_TABLE[] = {
    {335,  0.0f},
    {473,  0.5f},
    {596,  1.0f},
    {736,  1.5f},
    {862,  2.0f},
    {996,  2.5f},
    {1137, 3.0f},
    {1270, 3.5f},
    {1395, 4.0f},
    {1555, 4.5f},
    {1678, 5.0f},
    {1817, 5.5f},
    {1976, 6.0f}
};

constexpr size_t RETURN_PRESSURE_TABLE_SIZE =
    sizeof(RETURN_PRESSURE_TABLE) /
    sizeof(RETURN_PRESSURE_TABLE[0]);

constexpr uint8_t PRESSURE_CITY_PIN = 3;

const Config::PressureCalibrationPoint CITY_PRESSURE_TABLE[] = {
    {350,  0.0f},
    {480,  0.5f},
    {600,  1.0f},
    {750,  1.5f},
    {870,  2.0f},
    {1000, 2.5f},
    {1133, 3.0f},
    {1275, 3.5f},
    {1406, 4.0f},
    {1545, 4.5f},
    {1688, 5.0f},
    {1835, 5.5f},
    {2000, 6.0f},
};

constexpr size_t CITY_PRESSURE_TABLE_SIZE =
    sizeof(CITY_PRESSURE_TABLE) / sizeof(CITY_PRESSURE_TABLE[0]);

}
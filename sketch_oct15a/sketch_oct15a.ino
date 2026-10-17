#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "DisplayManager.h"
#include "MeasurementRegistry.h"
#include "TemperatureSensor.h"
#include "DisplayPages.h"
#include "AlarmManager.h"

// =====================================================
// DISPLAY SETTINGS
// =====================================================
constexpr uint8_t TEMP_LCD_ADDRESS = 0x25;
constexpr uint8_t PRESS_LCD_ADDRESS = 0x27;

// =====================================================
// MEASUREMENT PAGE CONFIGURATION
// =====================================================
const MeasurementField temperatureFields[] = {
    {ChannelId::TEMP_SUPPLY, "PODACHA", 1, "C", 0.001f},
    {ChannelId::TEMP_RETURN, "OBRATKA", 1, "C", 0.001f}
};

// =====================================================
// ALARM CONFIGURATION
// =====================================================
const AlarmChannelConfig alarmConfigs[] = {
    {ChannelId::TEMP_SUPPLY, Config::POD_TEMP_WARN_LOW, Config::POD_TEMP_CRITICAL_LOW, Config::POD_TEMP_WARN_HIGH, Config::POD_TEMP_CRITICAL_HIGH, Config::TEMP_HYSTERESIS},
    {ChannelId::TEMP_RETURN, Config::OBR_TEMP_WARN_LOW, Config::OBR_TEMP_CRITICAL_LOW, Config::OBR_TEMP_WARN_HIGH, Config::OBR_TEMP_CRITICAL_HIGH, Config::TEMP_HYSTERESIS}
};
constexpr uint8_t ALARM_CONFIG_COUNT = sizeof(alarmConfigs) / sizeof(alarmConfigs[0]);

// =====================================================
// MODULES
// =====================================================
LcdDisplay temperatureDisplay(TEMP_LCD_ADDRESS, 16, 2);
LcdDisplay pressureDisplay(PRESS_LCD_ADDRESS, 16, 2);
DisplayManager displayManager;
MeasurementRegistry measurementRegistry;
TemperatureSensor temperatureSensor(measurementRegistry);
DisplayPages displayPages(measurementRegistry, displayManager);
AlarmManager alarmManager(measurementRegistry, alarmConfigs, ALARM_CONFIG_COUNT);

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);
    displayManager.addDisplay(temperatureDisplay);
    displayManager.addDisplay(pressureDisplay);
    ScreenPage temperaturePage;
    temperaturePage.setLine(0, "PODACHA: ---");
    temperaturePage.setLine(1, "OBRATKA: ---");
    displayManager.addPage(temperatureDisplay, temperaturePage);
    ScreenPage pressurePage;
    pressurePage.setLine(0, "TYSK");
    pressurePage.setLine(1, "Ochikuvannya");
    displayManager.addPage(pressureDisplay, pressurePage);
    if (!displayPages.addMeasurementPage(temperatureDisplay, 0, temperatureFields, sizeof(temperatureFields) / sizeof(temperatureFields[0]))) {
        Serial.println("ERROR: temperature page registration failed");
    }
    temperatureSensor.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {
    temperatureSensor.service();
    alarmManager.service();
    displayPages.service();
    displayManager.service();
}
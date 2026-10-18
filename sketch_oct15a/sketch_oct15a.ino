#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "Channel.h"
#include "DisplayManager.h"
#include "DisplayPages.h"
#include "TemperatureSensor.h"
#include "AlarmManager.h"
#include "AlarmDisplay.h"

constexpr uint8_t TEMP_LCD_ADDRESS = 0x25;
constexpr uint8_t PRESS_LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;

Channel supplyTemperature("PODACHA", "C", 1, 0.1f);
Channel returnTemperature("OBRATKA", "C", 1, 0.1f);

const AlarmThresholds supplyTemperatureAlarms = {
    Config::POD_TEMP_NORMAL,
    Config::POD_TEMP_CRITICAL_LOW,
    Config::POD_TEMP_WARN_LOW,
    Config::POD_TEMP_WARN_HIGH,
    Config::POD_TEMP_CRITICAL_HIGH,
    Config::TEMP_HYSTERESIS,
    Config::POD_TEMP_RETRIGGER_STEP
};

const AlarmThresholds returnTemperatureAlarms = {
    Config::OBR_TEMP_NORMAL,
    Config::OBR_TEMP_CRITICAL_LOW,
    Config::OBR_TEMP_WARN_LOW,
    Config::OBR_TEMP_WARN_HIGH,
    Config::OBR_TEMP_CRITICAL_HIGH,
    Config::TEMP_HYSTERESIS,
    Config::OBR_TEMP_RETRIGGER_STEP
};

LcdDisplay temperatureDisplay(TEMP_LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
LcdDisplay pressureDisplay(PRESS_LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
DisplayManager displayManager;
DisplayPages displayPages(displayManager);
TemperatureSensor temperatureSensor;
AlarmManager alarmManager;
AlarmDisplay alarmDisplay(alarmManager, displayManager, displayPages);

bool setupTemperaturePage() {
    ScreenPage page;
    page.setLine(0, "PODACHA: ---");
    page.setLine(1, "ZVOROT: ---");
    if (!displayManager.addPage(temperatureDisplay, page)) return false;
    const char* const lines[] = {
        "{CHANNEL_NAME}: {VALUE}{UNIT}",
        "{CHANNEL_NAME}: {VALUE}{UNIT}"
    };
    if (!displayPages.addTemplatePage(temperatureDisplay, 0, lines, 2)) return false;
    if (!displayPages.bind(temperatureDisplay, 0, 0, supplyTemperature)) return false;
    if (!displayPages.bind(temperatureDisplay, 0, 1, returnTemperature)) return false;
    return true;
}

bool setupPressurePage() {
    ScreenPage page;
    page.setLine(0, "GOR:0.1 ATM");
    page.setLine(1, "POD:0.1 OBR:0.1");
    return displayManager.addPage(pressureDisplay, page);
}

bool initializeSystem() {
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);
    if (!displayManager.addDisplay(temperatureDisplay)) Serial.println("ERROR: temperature LCD registration failed");
    if (!displayManager.addDisplay(pressureDisplay)) Serial.println("ERROR: pressure LCD registration failed");
    if (!setupTemperaturePage()) Serial.println("ERROR: temperature page setup failed");
    if (!setupPressurePage()) Serial.println("ERROR: pressure page setup failed");
    if (!alarmManager.addChannel(supplyTemperature, supplyTemperatureAlarms)) Serial.println("ERROR: supply temperature alarm setup failed");
    if (!alarmManager.addChannel(returnTemperature, returnTemperatureAlarms)) Serial.println("ERROR: return temperature alarm setup failed");
    if (!temperatureSensor.addSensor(Config::TEMP_SUPPLY_ROM, supplyTemperature)) Serial.println("ERROR: supply temperature sensor registration failed");
    if (!temperatureSensor.addSensor(Config::TEMP_RETURN_ROM, returnTemperature)) Serial.println("ERROR: return temperature sensor registration failed");
    if (!alarmDisplay.addDisplay(temperatureDisplay)) Serial.println("ERROR: temperature alarm display setup failed");
    temperatureSensor.begin();
    Serial.println("Boiler monitoring system started");
    return true;
}

void setup() {
    Serial.begin(115200);
    initializeSystem();
}

void loop() {
    temperatureSensor.service();
    alarmManager.service();
    displayPages.service();
    alarmDisplay.service();
    displayManager.service();
}
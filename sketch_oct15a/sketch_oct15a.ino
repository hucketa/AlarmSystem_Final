#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "Channel.h"
#include "DisplayManager.h"
#include "DisplayPages.h"
#include "TemperatureSensor.h"
#include "PressureSensor.h"
#include "AlarmManager.h"
#include "BuzzerManager.h"
#include "AlarmDisplay.h"
#include "LedManager.h"

// =====================================================
// TEMPERATURE CHANNELS
// =====================================================
Channel supplyTemperature("PODACHA", "C", 1, 0.1f);
Channel returnTemperature("OBRATKA", "C", 1, 0.1f);

// =====================================================
// PRESSURE CHANNELS
// =====================================================
Channel cityPressure("GOR", "atm", 1, 0.01f);
Channel supplyPressure("POD", "atm", 1, 0.01f);
Channel returnPressure("OBR", "atm", 1, 0.01f);
Channel cityPressureRaw("RAW", "", 0, 1.0f);
Channel cityPressureMv("mV", "", 1, 1.0f);
Channel supplyPressureRaw("RAW", "", 0, 1.0f);
Channel supplyPressureMv("mV", "", 1, 1.0f);
Channel returnPressureRaw("RAW", "", 0, 1.0f);
Channel returnPressureMv("mV", "", 1, 1.0f);

// =====================================================
// TEMPERATURE ALARMS
// =====================================================
const AlarmThresholds supplyTemperatureAlarms = {Config::POD_TEMP_NORMAL, Config::POD_TEMP_CRITICAL_LOW, Config::POD_TEMP_WARN_LOW, Config::POD_TEMP_WARN_HIGH, Config::POD_TEMP_CRITICAL_HIGH, Config::TEMP_HYSTERESIS, Config::POD_TEMP_RETRIGGER_STEP};
const AlarmThresholds returnTemperatureAlarms = {Config::OBR_TEMP_NORMAL, Config::OBR_TEMP_CRITICAL_LOW, Config::OBR_TEMP_WARN_LOW, Config::OBR_TEMP_WARN_HIGH, Config::OBR_TEMP_CRITICAL_HIGH, Config::TEMP_HYSTERESIS, Config::OBR_TEMP_RETRIGGER_STEP};

// =====================================================
// PRESSURE ALARMS
// =====================================================
const AlarmThresholds cityPressureAlarms = {Config::CITY_PRESS_NORMAL, Config::CITY_PRESS_CRITICAL_LOW, Config::CITY_PRESS_WARN_LOW, Config::CITY_PRESS_WARN_HIGH, Config::CITY_PRESS_CRITICAL_HIGH, Config::PRESSURE_HYSTERESIS, Config::CITY_PRESS_RETRIGGER_STEP};
const AlarmThresholds supplyPressureAlarms = {Config::POD_PRESS_NORMAL, Config::POD_PRESS_CRITICAL_LOW, Config::POD_PRESS_WARN_LOW, Config::POD_PRESS_WARN_HIGH, Config::POD_PRESS_CRITICAL_HIGH, Config::PRESSURE_HYSTERESIS, Config::POD_PRESS_RETRIGGER_STEP};
const AlarmThresholds returnPressureAlarms = {Config::OBR_PRESS_NORMAL, Config::OBR_PRESS_CRITICAL_LOW, Config::OBR_PRESS_WARN_LOW, Config::OBR_PRESS_WARN_HIGH, Config::OBR_PRESS_CRITICAL_HIGH, Config::PRESSURE_HYSTERESIS, Config::OBR_PRESS_RETRIGGER_STEP};

// =====================================================
// DISPLAYS
// =====================================================
LcdDisplay temperatureDisplay(Config::TEMP_LCD_ADDRESS, Config::LCD_COLUMNS, Config::LCD_ROWS);
LcdDisplay pressureDisplay(Config::PRESS_LCD_ADDRESS, Config::LCD_COLUMNS, Config::LCD_ROWS);

// =====================================================
// MANAGERS
// =====================================================
DisplayManager displayManager;
DisplayPages displayPages(displayManager);
TemperatureSensor temperatureSensor;
AlarmManager alarmManager;
AlarmDisplay alarmDisplay(alarmManager, displayManager, displayPages);
BuzzerManager buzzerManager;
LedManager ledManager(alarmManager);

// =====================================================
// PRESSURE SENSORS
// =====================================================
PressureSensor supplyPressureSensor(Config::PRESSURE_SUPPLY_PIN, supplyPressure, supplyPressureRaw, supplyPressureMv, Config::SUPPLY_PRESSURE_TABLE, Config::SUPPLY_PRESSURE_TABLE_SIZE);
PressureSensor returnPressureSensor(Config::PRESSURE_RETURN_PIN, returnPressure, returnPressureRaw, returnPressureMv, Config::RETURN_PRESSURE_TABLE, Config::RETURN_PRESSURE_TABLE_SIZE);
PressureSensor cityPressureSensor(Config::PRESSURE_CITY_PIN, cityPressure, cityPressureRaw, cityPressureMv, Config::CITY_PRESSURE_TABLE, Config::CITY_PRESSURE_TABLE_SIZE);

// =====================================================
// TEMPERATURE DISPLAY
// =====================================================
bool setupTemperaturePage() {
    ScreenPage page;
    page.setLine(0, "PODACHA: ---");
    page.setLine(1, "OBRATKA: ---");
    if (!displayManager.addPage(temperatureDisplay, page)) return false;
    const char* const lines[] = {"{CHANNEL_NAME}: {VALUE}{UNIT}", "{CHANNEL_NAME}: {VALUE}{UNIT}"};
    if (!displayPages.addTemplatePage(temperatureDisplay, 0, lines, 2)) return false;
    if (!displayPages.bind(temperatureDisplay, 0, 0, supplyTemperature)) return false;
    if (!displayPages.bind(temperatureDisplay, 0, 1, returnTemperature)) return false;
    return true;
}

// =====================================================
// PRESSURE DISPLAY
// =====================================================
bool setupPressurePage() {
    ScreenPage page;
    page.setLine(0, "GOR:----");
    page.setLine(1, "POD:--- OBR:---");
    if (!displayManager.addPage(pressureDisplay, page)) return false;
    const char* const lines[] = {"GOR:{VALUE}", "{CHANNEL_NAME}:{VALUE} {CHANNEL_NAME}:{VALUE}"};
    if (!displayPages.addTemplatePage(pressureDisplay, 0, lines, 2)) return false;
    if (!displayPages.bind(pressureDisplay, 0, 0, cityPressure)) return false;
    if (!displayPages.bind(pressureDisplay, 0, 1, supplyPressure)) return false;
    if (!displayPages.bind(pressureDisplay, 0, 1, returnPressure)) return false;
    return true;
}

// =====================================================
// INITIALIZATION
// =====================================================
bool initializeSystem() {
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);
    if (!displayManager.addDisplay(temperatureDisplay)) Serial.println("ERROR: temperature LCD registration failed");
    if (!displayManager.addDisplay(pressureDisplay)) Serial.println("ERROR: pressure LCD registration failed");
    if (!setupTemperaturePage()) Serial.println("ERROR: temperature page setup failed");
    if (!setupPressurePage()) Serial.println("ERROR: pressure page setup failed");
    if (!alarmManager.addChannel(supplyTemperature, supplyTemperatureAlarms)) Serial.println("ERROR: supply temperature alarm setup failed");
    if (!alarmManager.addChannel(returnTemperature, returnTemperatureAlarms)) Serial.println("ERROR: return temperature alarm setup failed");
    if (!alarmManager.addChannel(cityPressure, cityPressureAlarms)) Serial.println("ERROR: city pressure alarm setup failed");
    if (!alarmManager.addChannel(supplyPressure, supplyPressureAlarms)) Serial.println("ERROR: supply pressure alarm setup failed");
    if (!alarmManager.addChannel(returnPressure, returnPressureAlarms)) Serial.println("ERROR: return pressure alarm setup failed");
    if (!temperatureSensor.addSensor(Config::TEMP_SUPPLY_ROM, supplyTemperature)) Serial.println("ERROR: supply temperature sensor registration failed");
    if (!temperatureSensor.addSensor(Config::TEMP_RETURN_ROM, returnTemperature)) Serial.println("ERROR: return temperature sensor registration failed");
    if (!alarmDisplay.addDisplay(temperatureDisplay)) Serial.println("ERROR: temperature alarm display setup failed");
    if (!alarmDisplay.addDisplay(pressureDisplay)) Serial.println("ERROR: pressure alarm display setup failed");
    temperatureSensor.begin();
    supplyPressureSensor.begin();
    returnPressureSensor.begin();
    cityPressureSensor.begin();
    Serial.println();
    Serial.println("===== BOILER MONITORING SYSTEM =====");
    Serial.println("Temperature LCD: initialized");
    Serial.println("Pressure LCD: initialized");
    Serial.println("Temperature alarms: 2");
    Serial.println("Pressure alarms: 3");
    Serial.println("Pressure GPIO: GOR=3, POD=0, OBR=1");
    Serial.println("Pressure calibration: Config.h");
    Serial.println("Serial Monitor: 115200 baud");
    Serial.println("====================================");
    return true;
}

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);
    buzzerManager.begin();
    ledManager.addChannel(LedGroup::TEMPERATURE, supplyTemperature);
    ledManager.addChannel(LedGroup::TEMPERATURE, returnTemperature);
    ledManager.addChannel(LedGroup::PRESSURE, cityPressure);
    ledManager.addChannel(LedGroup::PRESSURE, supplyPressure);
    ledManager.addChannel(LedGroup::PRESSURE, returnPressure);
    ledManager.begin();
    initializeSystem();
}

// =====================================================
// LOOP
// =====================================================
void loop() {
    temperatureSensor.service();
    supplyPressureSensor.service();
    returnPressureSensor.service();
    cityPressureSensor.service();
    alarmManager.service();
    buzzerManager.service();
    ledManager.service();
    displayPages.service();
    alarmDisplay.service();
    displayManager.service();
}
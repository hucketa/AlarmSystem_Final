
#include <Arduino.h>
#include <Wire.h>

#include "Config.h"
#include "DisplayManager.h"
#include "SensorTypes.h"
#include "MeasurementRegistry.h"



// =====================================================
// LCD
// =====================================================

LcdDisplay temperatureDisplay(0x25, 16, 2);
LcdDisplay pressureDisplay(0x27, 16, 2);

DisplayManager displayManager;
MeasurementRegistry measurementRegistry;

// =====================================================
// SETUP
// =====================================================

void setup() {
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);

    displayManager.addDisplay(temperatureDisplay);
    displayManager.addDisplay(pressureDisplay);

    ScreenPage temperaturePage;
    temperaturePage.setLine(0, "TEMPERATURA");
    temperaturePage.setLine(1, "Systema hotova");

    displayManager.addPage(
        temperatureDisplay,
        temperaturePage
    );

    ScreenPage pressurePage;
    pressurePage.setLine(0, "TYSK");
    pressurePage.setLine(1, "Systema hotova");

    displayManager.addPage(
        pressureDisplay,
        pressurePage
    );
}

// =====================================================
// LOOP
// =====================================================

void loop() {
    displayManager.service();
}

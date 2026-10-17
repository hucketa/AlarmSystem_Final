
#include <Arduino.h>
#include "Config.h"
#include "SensorTypes.h"
#include "MeasurementRegistry.h"
#include "TemperatureSensor.h"

// =====================================================
// TEST SETTINGS
// =====================================================
constexpr uint32_t SERIAL_INTERVAL_MS = 1000;

// =====================================================
// MEASUREMENTS AND SENSOR
// =====================================================
MeasurementRegistry measurementRegistry;
TemperatureSensor temperatureSensor(measurementRegistry);
uint32_t lastSerialOutputMs = 0;

// =====================================================
// PRINT MEASUREMENT
// =====================================================
void printMeasurement(ChannelId channel, const char* name) {
    Measurement measurement;
    Serial.printf("%s: ", name);
    if (!measurementRegistry.get(channel, measurement)) {
        Serial.println("CHANNEL NOT FOUND");
        return;
    }
    switch (measurement.status) {
        case MeasurementStatus::NOT_INITIALIZED:
            Serial.println("NOT INITIALIZED");
            return;
        case MeasurementStatus::ERROR:
            Serial.println("ERROR");
            return;
        case MeasurementStatus::VALID:
            Serial.printf("VALID | %.2f C | Age: %lu ms\n", measurement.value, (unsigned long)(millis() - measurement.lastUpdateMs));
            return;
    }
    Serial.println("UNKNOWN");
}

// =====================================================
// SETUP
// =====================================================
void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("================================");
    Serial.println(" TEMPERATURE MODULE TEST");
    Serial.println("================================");
    temperatureSensor.begin();
}

// =====================================================
// LOOP
// =====================================================
void loop() {
    temperatureSensor.service();
    uint32_t now = millis();
    if (now - lastSerialOutputMs >= SERIAL_INTERVAL_MS) {
        lastSerialOutputMs = now;
        printMeasurement(ChannelId::TEMP_SUPPLY, "Supply");
        printMeasurement(ChannelId::TEMP_RETURN, "Return");
        Serial.println();
    }
}

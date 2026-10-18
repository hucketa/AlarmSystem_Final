
#include "TemperatureSensor.h"
#include <math.h>
#include <string.h>

TemperatureSensor::TemperatureSensor(uint8_t pin)
    : oneWire(pin), sensors(&oneWire) {
}

bool TemperatureSensor::addSensor(const uint8_t rom[8], Channel& channel) {
    if (started || rom == nullptr || sensorCount >= MAX_SENSORS) {
        return false;
    }
    for (uint8_t i = 0; i < sensorCount; i++) {
        if (memcmp(entries[i].rom, rom, sizeof(entries[i].rom)) == 0) {
            return false;
        }
    }
    SensorEntry& entry = entries[sensorCount];
    memcpy(entry.rom, rom, sizeof(entry.rom));
    entry.channel = &channel;
    entry.configured = false;
    sensorCount++;
    return true;
}

void TemperatureSensor::begin() {
    sensors.begin();
    sensors.setWaitForConversion(false);
    started = true;
    for (uint8_t i = 0; i < sensorCount; i++) {
        if (configureSensor(entries[i])) {
            Serial.print("DS18B20 ");
            Serial.print(entries[i].channel->getName());
            Serial.println(": found");
        } else {
            entries[i].channel->setError(millis());
            Serial.print("DS18B20 ");
            Serial.print(entries[i].channel->getName());
            Serial.println(": NOT FOUND");
        }
    }
    startConversion(millis());
}

void TemperatureSensor::service() {
    if (!started) {
        return;
    }
    uint32_t now = millis();
    if (conversionPending && now - conversionStartedMs < Config::TEMP_CONVERSION_TIME_MS) {
        return;
    }
    if (conversionPending) {
        readMeasurements(now);
    }
    startConversion(now);
}

void TemperatureSensor::readMeasurements(uint32_t timestamp) {
    for (uint8_t i = 0; i < sensorCount; i++) {
        SensorEntry& entry = entries[i];
        if (!sensors.isConnected(entry.rom)) {
            entry.configured = false;
            entry.channel->setError(timestamp);
            continue;
        }
        if (!entry.configured) {
            configureSensor(entry);
            entry.channel->setError(timestamp);
            continue;
        }
        float temperature = sensors.getTempC(entry.rom);
        if (temperature != DEVICE_DISCONNECTED_C &&
            isfinite(temperature) &&
            temperature >= -55.0f &&
            temperature <= 125.0f) {
            entry.channel->updateValue(temperature, timestamp);
        } else {
            entry.channel->setError(timestamp);
        }
    }
}

void TemperatureSensor::startConversion(uint32_t timestamp) {
    sensors.requestTemperatures();
    conversionStartedMs = timestamp;
    conversionPending = true;
}

bool TemperatureSensor::configureSensor(SensorEntry& entry) {
    if (!sensors.isConnected(entry.rom)) {
        entry.configured = false;
        return false;
    }
    sensors.setResolution(entry.rom, Config::TEMP_RESOLUTION_BITS);
    entry.configured = true;
    return true;
}

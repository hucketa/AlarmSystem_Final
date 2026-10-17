#include "TemperatureSensor.h"
#include <math.h>

// =====================================================
// CONSTRUCTOR
// =====================================================
TemperatureSensor::TemperatureSensor(MeasurementRegistry& registry) : oneWire(Config::TEMP_PIN), sensors(&oneWire), registry(registry) {
}

// =====================================================
// INITIALIZE SENSORS
// =====================================================
void TemperatureSensor::begin() {
    sensors.begin();
    sensors.setWaitForConversion(false);
    bool supplyFound = sensors.isConnected(Config::TEMP_SUPPLY_ROM);
    bool returnFound = sensors.isConnected(Config::TEMP_RETURN_ROM);
    if (supplyFound) {
        sensors.setResolution(Config::TEMP_SUPPLY_ROM, Config::TEMP_RESOLUTION_BITS);
        Serial.println("DS18B20 supply: found");
    } else {
        registry.setError(ChannelId::TEMP_SUPPLY);
        Serial.println("DS18B20 supply: NOT FOUND");
    }
    if (returnFound) {
        sensors.setResolution(Config::TEMP_RETURN_ROM, Config::TEMP_RESOLUTION_BITS);
        Serial.println("DS18B20 return: found");
    } else {
        registry.setError(ChannelId::TEMP_RETURN);
        Serial.println("DS18B20 return: NOT FOUND");
    }
    initialized = supplyFound || returnFound;
    if (initialized) {
        startConversion(millis());
    } else {
        Serial.println("DS18B20: no configured sensors found");
    }
}

// =====================================================
// SERVICE SENSORS
// =====================================================
void TemperatureSensor::service() {
    if (!initialized) {
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

// =====================================================
// READ MEASUREMENTS
// =====================================================
void TemperatureSensor::readMeasurements(uint32_t timestamp) {
    float temperature;
    if (sensors.isConnected(Config::TEMP_SUPPLY_ROM)) {
        temperature = sensors.getTempC(Config::TEMP_SUPPLY_ROM);
        if (temperature != DEVICE_DISCONNECTED_C && isfinite(temperature) && temperature >= -55.0f && temperature <= 125.0f) {
            registry.update(ChannelId::TEMP_SUPPLY, temperature, timestamp);
        } else {
            registry.setError(ChannelId::TEMP_SUPPLY);
        }
    } else {
        registry.setError(ChannelId::TEMP_SUPPLY);
    }
    if (sensors.isConnected(Config::TEMP_RETURN_ROM)) {
        temperature = sensors.getTempC(Config::TEMP_RETURN_ROM);
        if (temperature != DEVICE_DISCONNECTED_C && isfinite(temperature) && temperature >= -55.0f && temperature <= 125.0f) {
            registry.update(ChannelId::TEMP_RETURN, temperature, timestamp);
        } else {
            registry.setError(ChannelId::TEMP_RETURN);
        }
    } else {
        registry.setError(ChannelId::TEMP_RETURN);
    }
}

// =====================================================
// START ASYNCHRONOUS CONVERSION
// =====================================================
void TemperatureSensor::startConversion(uint32_t timestamp) {
    conversionStartedMs = timestamp;
    sensors.requestTemperatures();
    conversionPending = true;
}
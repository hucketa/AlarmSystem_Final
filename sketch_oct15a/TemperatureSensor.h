
#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"
#include "SensorTypes.h"
#include "MeasurementRegistry.h"

class TemperatureSensor {
public:
    explicit TemperatureSensor(MeasurementRegistry& registry);
    void begin();
    void service();

private:
    OneWire oneWire;
    DallasTemperature sensors;
    MeasurementRegistry& registry;
    uint32_t conversionStartedMs = 0;
    bool conversionPending = false;
    bool initialized = false;

    void readMeasurements(uint32_t timestamp);
    void startConversion(uint32_t timestamp);
};

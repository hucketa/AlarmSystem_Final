
#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"
#include "Channel.h"

class TemperatureSensor {
public:
    explicit TemperatureSensor(uint8_t pin = Config::TEMP_PIN);
    bool addSensor(const uint8_t rom[8], Channel& channel);
    void begin();
    void service();

private:
    struct SensorEntry {
        uint8_t rom[8];
        Channel* channel = nullptr;
        bool configured = false;
    };
    static constexpr uint8_t MAX_SENSORS = 8;
    OneWire oneWire;
    DallasTemperature sensors;
    SensorEntry entries[MAX_SENSORS];
    uint8_t sensorCount = 0;
    uint32_t conversionStartedMs = 0;
    bool conversionPending = false;
    bool started = false;
    void readMeasurements(uint32_t timestamp);
    void startConversion(uint32_t timestamp);
    bool configureSensor(SensorEntry& entry);
};

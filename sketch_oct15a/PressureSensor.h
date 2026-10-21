
#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Channel.h"

class PressureSensor {
public:
    PressureSensor(
        uint8_t pin,
        Channel& pressureChannel,
        Channel& rawChannel,
        Channel& millivoltsChannel,
        const Config::PressureCalibrationPoint* table,
        size_t tableSize
    );

    bool begin();
    void service();

private:
    uint8_t pin;
    Channel& pressureChannel;
    Channel& rawChannel;
    Channel& millivoltsChannel;

    const Config::PressureCalibrationPoint* table;
    size_t tableSize;

    uint32_t nextSampleUs = 0;
    uint32_t rawSum = 0;
    uint32_t millivoltsSum = 0;
    uint8_t sampleCount = 0;
    bool started = false;

    bool validateTable() const;
    float rawToPressure(float raw) const;
    void finishMeasurement(uint32_t timestamp);
};

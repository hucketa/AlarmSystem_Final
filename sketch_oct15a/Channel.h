
#pragma once

#include <Arduino.h>

enum class MeasurementStatus : uint8_t {
    NOT_INITIALIZED,
    VALID,
    ERROR
};

class Channel {
public:
    Channel(const char* name, const char* unit, uint8_t precision, float changeStep);

    const char* getName() const;
    const char* getUnit() const;
    uint8_t getPrecision() const;
    float getChangeStep() const;

    float getValue() const;
    MeasurementStatus getStatus() const;
    uint32_t getLastUpdateMs() const;

    bool updateValue(float value, uint32_t timestamp);
    void setError(uint32_t timestamp);
    void setNotInitialized(uint32_t timestamp);

private:
    const char* name;
    const char* unit;
    uint8_t precision;
    float changeStep;

    float value;
    MeasurementStatus status;
    uint32_t lastUpdateMs;
};

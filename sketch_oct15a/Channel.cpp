
#include "Channel.h"
#include <math.h>

Channel::Channel(const char* name, const char* unit, uint8_t precision, float changeStep)
    : name(name),
      unit(unit),
      precision(precision),
      changeStep(changeStep),
      value(0.0f),
      status(MeasurementStatus::NOT_INITIALIZED),
      lastUpdateMs(0) {
}

const char* Channel::getName() const {
    return name;
}

const char* Channel::getUnit() const {
    return unit;
}

uint8_t Channel::getPrecision() const {
    return precision;
}

float Channel::getChangeStep() const {
    return changeStep;
}

float Channel::getValue() const {
    return value;
}

MeasurementStatus Channel::getStatus() const {
    return status;
}

uint32_t Channel::getLastUpdateMs() const {
    return lastUpdateMs;
}

bool Channel::updateValue(float newValue, uint32_t timestamp) {
    if (!isfinite(newValue)) {
        setError(timestamp);
        return false;
    }

    value = newValue;
    status = MeasurementStatus::VALID;
    lastUpdateMs = timestamp;
    return true;
}

void Channel::setError(uint32_t timestamp) {
    status = MeasurementStatus::ERROR;
    lastUpdateMs = timestamp;
}

void Channel::setNotInitialized(uint32_t timestamp) {
    status = MeasurementStatus::NOT_INITIALIZED;
    lastUpdateMs = timestamp;
}

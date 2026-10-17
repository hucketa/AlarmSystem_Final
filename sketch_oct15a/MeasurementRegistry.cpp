
#include "MeasurementRegistry.h"
#include <math.h>

// =====================================================
// CONSTRUCTOR
// =====================================================

MeasurementRegistry::MeasurementRegistry() : measurements{} {
}

// =====================================================
// UPDATE MEASUREMENT
// =====================================================

bool MeasurementRegistry::update(ChannelId channel, float value, uint32_t timestamp) {
    int8_t index = indexOf(channel);
    if (index < 0 || !isfinite(value)) {
        return false;
    }
    Measurement& measurement = measurements[index];
    measurement.value = value;
    measurement.status = MeasurementStatus::VALID;
    measurement.lastUpdateMs = timestamp;
    return true;
}

// =====================================================
// SET CHANNEL ERROR
// =====================================================

bool MeasurementRegistry::setError(ChannelId channel) {
    int8_t index = indexOf(channel);
    if (index < 0) {
        return false;
    }
    measurements[index].status = MeasurementStatus::ERROR;
    return true;
}

// =====================================================
// GET MEASUREMENT
// =====================================================

bool MeasurementRegistry::get(ChannelId channel, Measurement& result) const {
    int8_t index = indexOf(channel);
    if (index < 0) {
        return false;
    }
    result = measurements[index];
    return true;
}

// =====================================================
// CHANNEL COUNT
// =====================================================

uint8_t MeasurementRegistry::channelCount() const {
    return CHANNEL_COUNT;
}

// =====================================================
// CHANNEL INDEX
// =====================================================

int8_t MeasurementRegistry::indexOf(ChannelId channel) {
    uint8_t index = static_cast<uint8_t>(channel);
    if (index >= CHANNEL_COUNT) {
        return -1;
    }
    return static_cast<int8_t>(index);
}

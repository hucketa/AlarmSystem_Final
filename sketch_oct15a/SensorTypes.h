
#pragma once

#include <Arduino.h>

// =====================================================
// IDENTIFIERS OF MEASUREMENT CHANNELS
// =====================================================

enum class ChannelId : uint8_t {
    TEMP_SUPPLY,
    TEMP_RETURN,
    PRESS_CITY,
    PRESS_SUPPLY,
    PRESS_RETURN,
    COUNT
};

// =====================================================
// MEASUREMENT STATUS
// =====================================================

enum class MeasurementStatus : uint8_t {
    NOT_INITIALIZED,
    VALID,
    ERROR
};

// =====================================================
// MEASUREMENT DATA
// =====================================================

struct Measurement {
    float value = 0.0f;
    MeasurementStatus status = MeasurementStatus::NOT_INITIALIZED;
    uint32_t lastUpdateMs = 0;
};

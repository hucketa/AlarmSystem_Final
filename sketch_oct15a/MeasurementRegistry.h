
#pragma once

#include <Arduino.h>
#include "SensorTypes.h"

class MeasurementRegistry {
public:
    MeasurementRegistry();
    bool update(ChannelId channel, float value, uint32_t timestamp);
    bool setError(ChannelId channel);
    bool get(ChannelId channel, Measurement& result) const;
    uint8_t channelCount() const;
private:
    static constexpr uint8_t CHANNEL_COUNT = static_cast<uint8_t>(ChannelId::COUNT);
    Measurement measurements[CHANNEL_COUNT];
    static int8_t indexOf(ChannelId channel);
};

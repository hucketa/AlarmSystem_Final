#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Channel.h"

enum class AlarmLevel : uint8_t {
    NORMAL,
    WARNING,
    CRITICAL,
    SENSOR_ERROR
};


enum class AlarmDirection : uint8_t {
    NONE,
    LOW_SIDE,
    HIGH_SIDE,
    SENSOR_ERROR
};


struct AlarmThresholds {
    float normal;
    float lowCritical;
    float lowWarning;
    float highWarning;
    float highCritical;
    float hysteresis;
    float retriggerStep;
};

struct AlarmInfo {
    Channel* channel = nullptr;
    AlarmLevel level = AlarmLevel::NORMAL;
    AlarmDirection direction = AlarmDirection::NONE;
    float value = 0.0f;
    uint32_t changedAtMs = 0;
    uint32_t triggerCount = 0;
    bool active = false;
};

class AlarmManager {
public:
    static constexpr uint8_t MAX_CHANNELS = Config::MAX_ALARM_CHANNELS;

    bool addChannel(Channel& channel, const AlarmThresholds& thresholds);
    void service();

    bool getAlarm(const Channel& channel, AlarmInfo& result) const;
    uint8_t activeAlarmCount() const;
    bool getActiveAlarm(uint8_t index, AlarmInfo& result) const;

    uint32_t getTriggerCount() const;
    bool hasNewTrigger() const;
    void acknowledgeTriggers();

private:
    struct AlarmEntry {
        Channel* channel = nullptr;
        AlarmThresholds thresholds = {};
        AlarmInfo alarm = {};
        float lastTriggeredValue = 0.0f;
        bool hasTriggeredValue = false;
    };

    AlarmEntry entries[MAX_CHANNELS] = {};
    uint8_t entryCount = 0;
    uint32_t triggerCount = 0;
    uint32_t acknowledgedTriggerCount = 0;

    int8_t findChannel(const Channel& channel) const;
    bool thresholdsValid(const AlarmThresholds& thresholds) const;

    void evaluate(AlarmEntry& entry, uint32_t now);
    void updateAlarm(
        AlarmEntry& entry,
        AlarmLevel level,
        AlarmDirection direction,
        float value,
        uint32_t now
    );
    void registerTrigger(AlarmEntry& entry, float value);
};
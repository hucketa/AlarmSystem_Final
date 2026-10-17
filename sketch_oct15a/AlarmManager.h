#pragma once
#include <Arduino.h>
#include "MeasurementRegistry.h"

// =====================================================
// ALARM STATE
// =====================================================
enum class AlarmState : uint8_t {
    NONE,
    LOW_ALARM,
    HIGH_ALARM,
    INVALID
};

// =====================================================
// ALARM SEVERITY
// =====================================================
enum class AlarmLevel : uint8_t {
    NONE,
    WARNING,
    CRITICAL
};

// =====================================================
// CHANNEL ALARM CONFIGURATION
// =====================================================
struct AlarmChannelConfig {
    ChannelId channel;
    float warningLow;
    float criticalLow;
    float warningHigh;
    float criticalHigh;
    float hysteresis;
};

// =====================================================
// ALARM INFORMATION
// =====================================================
struct AlarmInfo {
    ChannelId channel = ChannelId::TEMP_SUPPLY;
    AlarmState state = AlarmState::NONE;
    AlarmLevel level = AlarmLevel::NONE;
    float value = 0.0f;
    uint32_t changedAtMs = 0;
};

// =====================================================
// ALARM MANAGER
// =====================================================
class AlarmManager {
public:
    AlarmManager(MeasurementRegistry& registry, const AlarmChannelConfig* configs, uint8_t configCount);
    void service();
    bool getAlarm(ChannelId channel, AlarmInfo& result) const;
    uint8_t activeAlarmCount() const;
    bool getActiveAlarm(uint8_t index, AlarmInfo& result) const;
private:
    static constexpr uint8_t CHANNEL_COUNT = static_cast<uint8_t>(ChannelId::COUNT);
    MeasurementRegistry& registry;
    const AlarmChannelConfig* configs;
    uint8_t configCount;
    AlarmInfo alarms[CHANNEL_COUNT];
    static int8_t indexOf(ChannelId channel);
    bool isConfigured(ChannelId channel) const;
    void evaluateChannel(const AlarmChannelConfig& config, uint32_t now);
    void updateAlarm(ChannelId channel, AlarmState state, AlarmLevel level, float value, uint32_t now);
};
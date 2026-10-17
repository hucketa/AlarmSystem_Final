#include "AlarmManager.h"
#include <math.h>

// =====================================================
// CONSTRUCTOR
// =====================================================
AlarmManager::AlarmManager(MeasurementRegistry& registry, const AlarmChannelConfig* configs, uint8_t configCount)
    : registry(registry), configs(configs), configCount(configs == nullptr ? 0 : configCount), alarms{} {
    for (uint8_t i = 0; i < CHANNEL_COUNT; i++) {
        alarms[i].channel = static_cast<ChannelId>(i);
    }
}

// =====================================================
// SERVICE
// =====================================================
void AlarmManager::service() {
    uint32_t now = millis();
    for (uint8_t i = 0; i < configCount; i++) {
        const AlarmChannelConfig& config = configs[i];
        if (!isConfigured(config.channel)) continue;
        if (!isfinite(config.warningLow) || !isfinite(config.criticalLow) || !isfinite(config.warningHigh) || !isfinite(config.criticalHigh) || !isfinite(config.hysteresis)) continue;
        if (config.criticalLow > config.warningLow || config.warningLow >= config.warningHigh || config.warningHigh > config.criticalHigh || config.hysteresis < 0.0f) continue;
        evaluateChannel(config, now);
    }
}

// =====================================================
// EVALUATE CHANNEL
// =====================================================
void AlarmManager::evaluateChannel(const AlarmChannelConfig& config, uint32_t now) {
    Measurement measurement;
    if (!registry.get(config.channel, measurement)) return;
    if (measurement.status == MeasurementStatus::NOT_INITIALIZED) return;
    if (measurement.status == MeasurementStatus::ERROR) {
        updateAlarm(config.channel, AlarmState::INVALID, AlarmLevel::CRITICAL, measurement.value, now);
        return;
    }
    float warningLow = config.warningLow;
    float criticalLow = config.criticalLow;
    float warningHigh = config.warningHigh;
    float criticalHigh = config.criticalHigh;
    AlarmInfo previous;
    if (getAlarm(config.channel, previous)) {
        if (previous.state == AlarmState::LOW_ALARM) {
            warningLow += config.hysteresis;
            criticalLow += config.hysteresis;
        } else if (previous.state == AlarmState::HIGH_ALARM) {
            warningHigh -= config.hysteresis;
            criticalHigh -= config.hysteresis;
        }
    }
    if (measurement.value < warningLow) {
        AlarmLevel level = measurement.value <= criticalLow ? AlarmLevel::CRITICAL : AlarmLevel::WARNING;
        updateAlarm(config.channel, AlarmState::LOW_ALARM, level, measurement.value, now);
    } else if (measurement.value > warningHigh) {
        AlarmLevel level = measurement.value >= criticalHigh ? AlarmLevel::CRITICAL : AlarmLevel::WARNING;
        updateAlarm(config.channel, AlarmState::HIGH_ALARM, level, measurement.value, now);
    } else {
        updateAlarm(config.channel, AlarmState::NONE, AlarmLevel::NONE, measurement.value, now);
    }
}

// =====================================================
// UPDATE ALARM
// =====================================================
void AlarmManager::updateAlarm(ChannelId channel, AlarmState state, AlarmLevel level, float value, uint32_t now) {
    int8_t index = indexOf(channel);
    if (index < 0) return;
    AlarmInfo& alarm = alarms[index];
    if (alarm.state != state || alarm.level != level) alarm.changedAtMs = now;
    alarm.channel = channel;
    alarm.state = state;
    alarm.level = level;
    alarm.value = value;
}

// =====================================================
// GET ALARM
// =====================================================
bool AlarmManager::getAlarm(ChannelId channel, AlarmInfo& result) const {
    int8_t index = indexOf(channel);
    if (index < 0 || !isConfigured(channel)) return false;
    result = alarms[index];
    return true;
}

// =====================================================
// COUNT ACTIVE ALARMS
// =====================================================
uint8_t AlarmManager::activeAlarmCount() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < configCount; i++) {
        AlarmInfo alarm;
        if (getAlarm(configs[i].channel, alarm) && alarm.level != AlarmLevel::NONE) count++;
    }
    return count;
}

// =====================================================
// GET ACTIVE ALARM BY INDEX
// =====================================================
bool AlarmManager::getActiveAlarm(uint8_t index, AlarmInfo& result) const {
    uint8_t current = 0;
    for (uint8_t i = 0; i < configCount; i++) {
        AlarmInfo alarm;
        if (!getAlarm(configs[i].channel, alarm) || alarm.level == AlarmLevel::NONE) continue;
        if (current == index) {
            result = alarm;
            return true;
        }
        current++;
    }
    return false;
}

// =====================================================
// CHECK CHANNEL CONFIGURATION
// =====================================================
bool AlarmManager::isConfigured(ChannelId channel) const {
    if (static_cast<uint8_t>(channel) >= CHANNEL_COUNT) return false;
    for (uint8_t i = 0; i < configCount; i++) {
        if (configs[i].channel == channel) return true;
    }
    return false;
}

// =====================================================
// CHANNEL INDEX
// =====================================================
int8_t AlarmManager::indexOf(ChannelId channel) {
    uint8_t index = static_cast<uint8_t>(channel);
    if (index >= CHANNEL_COUNT) return -1;
    return static_cast<int8_t>(index);
}
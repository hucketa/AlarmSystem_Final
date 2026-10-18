#include "AlarmManager.h"
#include <math.h>

bool AlarmManager::addChannel(Channel& channel, const AlarmThresholds& thresholds) {
    if (entryCount >= MAX_CHANNELS || findChannel(channel) >= 0 || !thresholdsValid(thresholds)) return false;
    AlarmEntry& entry = entries[entryCount++];
    entry.channel = &channel;
    entry.thresholds = thresholds;
    entry.alarm.channel = &channel;
    entry.alarm.level = AlarmLevel::NORMAL;
    entry.alarm.direction = AlarmDirection::NONE;
    entry.alarm.value = channel.getValue();
    entry.alarm.active = false;
    return true;
}

void AlarmManager::service() {
    const uint32_t now = millis();
    for (uint8_t i = 0; i < entryCount; i++) evaluate(entries[i], now);
}

void AlarmManager::evaluate(AlarmEntry& entry, uint32_t now) {
    Channel& channel = *entry.channel;
    const AlarmInfo& previous = entry.alarm;
    const MeasurementStatus status = channel.getStatus();
    if (status == MeasurementStatus::NOT_INITIALIZED) return;
    if (status == MeasurementStatus::ERROR) {
        updateAlarm(entry, AlarmLevel::SENSOR_ERROR, AlarmDirection::SENSOR_ERROR, channel.getValue(), now);
        return;
    }
    const AlarmThresholds& t = entry.thresholds;
    const float value = channel.getValue();
    AlarmLevel level = AlarmLevel::NORMAL;
    AlarmDirection direction = AlarmDirection::NONE;
    if (value <= t.lowCritical) {
        level = AlarmLevel::CRITICAL;
        direction = AlarmDirection::LOW_SIDE;
    } else if (previous.direction == AlarmDirection::LOW_SIDE && previous.level == AlarmLevel::CRITICAL && value <= t.lowCritical + t.hysteresis) {
        level = AlarmLevel::CRITICAL;
        direction = AlarmDirection::LOW_SIDE;
    } else if (value < t.lowWarning) {
        level = AlarmLevel::WARNING;
        direction = AlarmDirection::LOW_SIDE;
    } else if (previous.direction == AlarmDirection::LOW_SIDE && previous.level == AlarmLevel::WARNING && value < t.lowWarning + t.hysteresis) {
        level = AlarmLevel::WARNING;
        direction = AlarmDirection::LOW_SIDE;
    }
    if (direction == AlarmDirection::NONE) {
        if (value >= t.highCritical) {
            level = AlarmLevel::CRITICAL;
            direction = AlarmDirection::HIGH_SIDE;
        } else if (previous.direction == AlarmDirection::HIGH_SIDE && previous.level == AlarmLevel::CRITICAL && value >= t.highCritical - t.hysteresis) {
            level = AlarmLevel::CRITICAL;
            direction = AlarmDirection::HIGH_SIDE;
        } else if (value > t.highWarning) {
            level = AlarmLevel::WARNING;
            direction = AlarmDirection::HIGH_SIDE;
        } else if (previous.direction == AlarmDirection::HIGH_SIDE && previous.level == AlarmLevel::WARNING && value > t.highWarning - t.hysteresis) {
            level = AlarmLevel::WARNING;
            direction = AlarmDirection::HIGH_SIDE;
        }
    }
    updateAlarm(entry, level, direction, value, now);
}

void AlarmManager::updateAlarm(AlarmEntry& entry, AlarmLevel level, AlarmDirection direction, float value, uint32_t now) {
    AlarmInfo& alarm = entry.alarm;
    const bool stateChanged = level != alarm.level || direction != alarm.direction;
    if (stateChanged) alarm.changedAtMs = now;
    alarm.level = level;
    alarm.direction = direction;
    alarm.value = value;
    alarm.active = level != AlarmLevel::NORMAL;
    if (!alarm.active) {
        entry.hasTriggeredValue = false;
        return;
    }
    bool trigger = stateChanged;
    const float step = entry.thresholds.retriggerStep;
    if (!trigger && entry.hasTriggeredValue && step > 0.0f && level != AlarmLevel::SENSOR_ERROR) {
        const float difference = value - entry.lastTriggeredValue;
        trigger = (direction == AlarmDirection::LOW_SIDE && difference <= -step) || (direction == AlarmDirection::HIGH_SIDE && difference >= step);
    }
    if (trigger) registerTrigger(entry, value);
}

void AlarmManager::registerTrigger(AlarmEntry& entry, float value) {
    triggerCount++;
    entry.alarm.triggerCount++;
    entry.lastTriggeredValue = value;
    entry.hasTriggeredValue = true;
}

bool AlarmManager::getAlarm(const Channel& channel, AlarmInfo& result) const {
    const int8_t index = findChannel(channel);
    if (index < 0) return false;
    result = entries[index].alarm;
    return true;
}

uint8_t AlarmManager::activeAlarmCount() const {
    uint8_t count = 0;
    for (uint8_t i = 0; i < entryCount; i++) {
        if (entries[i].alarm.active) count++;
    }
    return count;
}

bool AlarmManager::getActiveAlarm(uint8_t index, AlarmInfo& result) const {
    uint8_t current = 0;
    for (uint8_t i = 0; i < entryCount; i++) {
        if (!entries[i].alarm.active) continue;
        if (current == index) {
            result = entries[i].alarm;
            return true;
        }
        current++;
    }
    return false;
}

uint32_t AlarmManager::getTriggerCount() const {
    return triggerCount;
}

bool AlarmManager::hasNewTrigger() const {
    return triggerCount != acknowledgedTriggerCount;
}

void AlarmManager::acknowledgeTriggers() {
    acknowledgedTriggerCount = triggerCount;
}

int8_t AlarmManager::findChannel(const Channel& channel) const {
    for (uint8_t i = 0; i < entryCount; i++) {
        if (entries[i].channel == &channel) return static_cast<int8_t>(i);
    }
    return -1;
}

bool AlarmManager::thresholdsValid(const AlarmThresholds& t) const {
    if (!isfinite(t.normal) || !isfinite(t.lowCritical) || !isfinite(t.lowWarning) || !isfinite(t.highWarning) || !isfinite(t.highCritical) || !isfinite(t.hysteresis) || !isfinite(t.retriggerStep)) return false;
    return t.lowCritical < t.lowWarning && t.lowWarning < t.normal && t.normal < t.highWarning && t.highWarning < t.highCritical && t.hysteresis >= 0.0f && t.retriggerStep >= 0.0f;
}
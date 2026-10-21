#include "LedManager.h"

bool LedManager::addChannel(LedGroup group, Channel& channel) {
    LedState* led = nullptr;
    if (group == LedGroup::TEMPERATURE) led = &temperatureLed;
    else if (group == LedGroup::PRESSURE) led = &pressureLed;
    if (led == nullptr || led->channelCount >= MAX_CHANNELS_PER_LED) return false;
    for (uint8_t i = 0; i < led->channelCount; i++) {
        if (led->channels[i] == &channel) return false;
    }
    led->channels[led->channelCount++] = &channel;
    return true;
}

void LedManager::begin() {
    pinMode(temperatureLed.pin, OUTPUT);
    pinMode(pressureLed.pin, OUTPUT);
    digitalWrite(temperatureLed.pin, LOW);
    digitalWrite(pressureLed.pin, LOW);
    temperatureLed.blinkState = false;
    pressureLed.blinkState = false;
    temperatureLed.lastBlinkAt = millis();
    pressureLed.lastBlinkAt = millis();
}

void LedManager::service() {
    const uint32_t now = millis();
    updateLed(temperatureLed, now);
    updateLed(pressureLed, now);
}

void LedManager::updateLed(LedState& led, uint32_t now) {
    bool hasWarning = false;
    bool hasCritical = false;
    for (uint8_t i = 0; i < led.channelCount; i++) {
        AlarmInfo alarm;
        if (!alarmManager.getAlarm(*led.channels[i], alarm) || !alarm.active) continue;
        if (alarm.level == AlarmLevel::WARNING) hasWarning = true;
        if (alarm.level == AlarmLevel::CRITICAL || alarm.level == AlarmLevel::SENSOR_ERROR) hasCritical = true;
    }
    if (hasWarning && !hasCritical) {
        if (now - led.lastBlinkAt >= Config::LED_BLINK_INTERVAL_MS) {
            led.lastBlinkAt = now;
            led.blinkState = !led.blinkState;
        }
    } else {
        led.blinkState = false;
        led.lastBlinkAt = now;
    }
    const bool ledOn = hasCritical || (hasWarning && led.blinkState);
    digitalWrite(led.pin, ledOn ? HIGH : LOW);
}
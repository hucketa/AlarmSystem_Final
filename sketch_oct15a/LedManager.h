#pragma once
#include <Arduino.h>
#include "Config.h"
#include "AlarmManager.h"
#include "Channel.h"

enum class LedGroup : uint8_t {
    TEMPERATURE,
    PRESSURE
};

class LedManager {
public:
    explicit LedManager(AlarmManager& manager) : alarmManager(manager) {}
    bool addChannel(LedGroup group, Channel& channel);
    void begin();
    void service();

private:
    static constexpr uint8_t MAX_CHANNELS_PER_LED = 5;

    struct LedState {
        uint8_t pin;
        Channel* channels[MAX_CHANNELS_PER_LED] = {};
        uint8_t channelCount = 0;
        bool blinkState = false;
        uint32_t lastBlinkAt = 0;
    };

    AlarmManager& alarmManager;
    LedState temperatureLed = {Config::TEMP_LED_PIN};
    LedState pressureLed = {Config::PRESSURE_LED_PIN};

    void updateLed(LedState& led, uint32_t now);
};
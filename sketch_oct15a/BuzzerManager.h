#pragma once
#include <Arduino.h>
#include "Config.h"
#include "AlarmManager.h"

class BuzzerManager {
public:
    void begin();
    void service();
    bool consumeTriplePress();

private:
    static constexpr uint32_t MULTI_CLICK_INTERVAL_MS = 600;
    bool muted = false;
    bool buttonRawState = HIGH;
    bool buttonStableState = HIGH;
    bool triplePressDetected = false;
    uint8_t clickCount = 0;
    uint32_t buttonChangedAt = 0;
    uint32_t lastClickAt = 0;

    void serviceButton(uint32_t now);
    void updateBuzzer();
};
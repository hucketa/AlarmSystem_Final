#pragma once

#include <Arduino.h>
#include "Config.h"
#include "AlarmManager.h"

class BuzzerManager {
public:
    void begin();
    void service();

private:
    bool muted = false;
    bool buttonRawState = HIGH;
    bool buttonStableState = HIGH;
    uint32_t buttonChangedAt = 0;

    void serviceButton(uint32_t now);
    void updateBuzzer();
};
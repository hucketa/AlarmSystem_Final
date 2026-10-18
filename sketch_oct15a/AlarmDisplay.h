#pragma once

#include <Arduino.h>
#include "Config.h"
#include "AlarmManager.h"
#include "DisplayManager.h"
#include "DisplayPages.h"

class AlarmDisplay {
public:
    AlarmDisplay(
        AlarmManager& alarms,
        DisplayManager& manager,
        DisplayPages& pages
    );

    bool addDisplay(LcdDisplay& display);
    void service();

private:
    struct DisplaySlot {
        LcdDisplay* display = nullptr;
        uint8_t normalPageCount = 0;
        uint8_t normalPageIndex = 0;
        uint8_t alarmPageIndex = 0;
        uint8_t alarmOrdinal = 0;
        Channel* currentAlarmChannel = nullptr;
        uint32_t lastSwitchMs = 0;
        bool showingAlarm = false;
        bool initialized = false;
    };

    AlarmManager& alarms;
    DisplayManager& manager;
    DisplayPages& pages;

    DisplaySlot displays[Config::MAX_DISPLAYS] = {};
    uint8_t displayCount = 0;

    int8_t findDisplay(const LcdDisplay& display) const;
    uint8_t countDisplayAlarms(const LcdDisplay& display) const;

    bool getDisplayAlarm(
        const LcdDisplay& display,
        uint8_t ordinal,
        AlarmInfo& result
    ) const;

    void showNormalPage(DisplaySlot& slot);
    void showAlarmPage(DisplaySlot& slot, const AlarmInfo& alarm);
    void buildAlarmPage(const AlarmInfo& alarm, ScreenPage& page) const;
};
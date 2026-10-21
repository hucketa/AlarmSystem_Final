#include "AlarmDisplay.h"
#include <stdio.h>
#include <string.h>

AlarmDisplay::AlarmDisplay(AlarmManager& alarms, DisplayManager& manager, DisplayPages& pages) : alarms(alarms), manager(manager), pages(pages) {
}

bool AlarmDisplay::addDisplay(LcdDisplay& display) {
    if (findDisplay(display) >= 0 || displayCount >= Config::MAX_DISPLAYS) return false;
    int8_t pageCount = manager.getPageCount(display);
    if (pageCount <= 0 || pageCount >= Config::MAX_PAGES_PER_DISPLAY) return false;
    ScreenPage alarmPage;
    if (!manager.addPage(display, alarmPage)) return false;
    DisplaySlot& slot = displays[displayCount++];
    slot.display = &display;
    slot.normalPageCount = static_cast<uint8_t>(pageCount);
    slot.normalPageIndex = 0;
    slot.alarmPageIndex = static_cast<uint8_t>(pageCount);
    slot.alarmOrdinal = 0;
    slot.lastSwitchMs = millis();
    slot.showingAlarm = false;
    slot.initialized = false;
    return true;
}

void AlarmDisplay::service() {
    const uint32_t now = millis();
    for (uint8_t i = 0; i < displayCount; i++) {
        DisplaySlot& slot = displays[i];
        const uint8_t alarmCount = countDisplayAlarms(*slot.display);
        if (!slot.initialized) {
            showNormalPage(slot);
            slot.initialized = true;
            slot.lastSwitchMs = now;
            continue;
        }
        if (slot.showingAlarm) {
            AlarmInfo currentAlarm;
            if (slot.currentAlarmChannel == nullptr || !alarms.getAlarm(*slot.currentAlarmChannel, currentAlarm) || !currentAlarm.active || !pages.isChannelBoundToDisplay(*slot.display, *slot.currentAlarmChannel)) {
                slot.showingAlarm = false;
                slot.currentAlarmChannel = nullptr;
                showNormalPage(slot);
                slot.lastSwitchMs = now;
                continue;
            }
            showAlarmPage(slot, currentAlarm);
        }
        if (now - slot.lastSwitchMs < Config::ALARM_SCREEN_TIME) continue;
        slot.lastSwitchMs = now;
        if (alarmCount == 0) {
            if (slot.showingAlarm) {
                slot.showingAlarm = false;
                slot.currentAlarmChannel = nullptr;
                showNormalPage(slot);
            } else if (slot.normalPageCount > 1) {
                slot.normalPageIndex++;
                if (slot.normalPageIndex >= slot.normalPageCount) slot.normalPageIndex = 0;
                showNormalPage(slot);
            }
            slot.alarmOrdinal = 0;
            continue;
        }
        if (slot.showingAlarm) {
            slot.showingAlarm = false;
            slot.currentAlarmChannel = nullptr;
            slot.normalPageIndex++;
            if (slot.normalPageIndex >= slot.normalPageCount) slot.normalPageIndex = 0;
            slot.alarmOrdinal++;
            if (slot.alarmOrdinal >= alarmCount) slot.alarmOrdinal = 0;
            showNormalPage(slot);
            continue;
        }
        slot.alarmOrdinal %= alarmCount;
        AlarmInfo alarm;
        if (getDisplayAlarm(*slot.display, slot.alarmOrdinal, alarm)) {
            showAlarmPage(slot, alarm);
            slot.showingAlarm = true;
            slot.currentAlarmChannel = alarm.channel;
        }
    }
}

int8_t AlarmDisplay::findDisplay(const LcdDisplay& display) const {
    for (uint8_t i = 0; i < displayCount; i++) {
        if (displays[i].display == &display) return static_cast<int8_t>(i);
    }
    return -1;
}

uint8_t AlarmDisplay::countDisplayAlarms(const LcdDisplay& display) const {
    uint8_t count = 0;
    AlarmInfo alarm;
    for (uint8_t i = 0; i < alarms.activeAlarmCount(); i++) {
        if (alarms.getActiveAlarm(i, alarm) && alarm.channel != nullptr && pages.isChannelBoundToDisplay(display, *alarm.channel)) count++;
    }
    return count;
}

bool AlarmDisplay::getDisplayAlarm(const LcdDisplay& display, uint8_t ordinal, AlarmInfo& result) const {
    uint8_t current = 0;
    AlarmInfo alarm;
    for (uint8_t i = 0; i < alarms.activeAlarmCount(); i++) {
        if (!alarms.getActiveAlarm(i, alarm) || alarm.channel == nullptr || !pages.isChannelBoundToDisplay(display, *alarm.channel)) continue;
        if (current == ordinal) {
            result = alarm;
            return true;
        }
        current++;
    }
    return false;
}

void AlarmDisplay::showNormalPage(DisplaySlot& slot) {
    manager.showPage(*slot.display, slot.normalPageIndex);
}

void AlarmDisplay::showAlarmPage(DisplaySlot& slot, const AlarmInfo& alarm) {
    ScreenPage page;
    buildAlarmPage(alarm, page);
    if (manager.replacePage(*slot.display, slot.alarmPageIndex, page)) manager.showPage(*slot.display, slot.alarmPageIndex);
}

void AlarmDisplay::buildAlarmPage(const AlarmInfo& alarm, ScreenPage& page) const {
    if (alarm.channel == nullptr) {
        page.setLine(0, "POMYLKA ALARMU");
        page.setLine(1, "NEMA KANALU");
        return;
    }
    const Channel& channel = *alarm.channel;
    const char* level = alarm.level == AlarmLevel::WARNING ? "UVAGA" : "OPASNO";
    if (alarm.direction == AlarmDirection::SENSOR_ERROR) {
        page.setLine(0, "POMYLKA VYMIRU");
        page.setLine(1, channel.getName());
        return;
    }
    const char* direction = "";
    switch (alarm.direction) {
        case AlarmDirection::LOW_SIDE:
            direction = "NYZKO";
            break;
        case AlarmDirection::HIGH_SIDE:
            direction = "VYSOKO";
            break;
        default:
            direction = "";
            break;
    }
    char line[Config::MAX_LCD_COLS + 1];
    snprintf(line, sizeof(line), "%s: %s", channel.getName(), level);
    page.setLine(0, line);
    char valueText[Config::MAX_LCD_COLS + 1];
    pages.formatValueWithUnit(channel, alarm.value, valueText, sizeof(valueText));
    snprintf(line, sizeof(line), "%s %s", direction, valueText);
    page.setLine(1, line);
}
#pragma once
#include <Arduino.h>
#include "Config.h"
#include "DisplayManager.h"
#include "MeasurementRegistry.h"

struct MeasurementField {
    ChannelId channel;
    const char* label;
    uint8_t precision;
    const char* unit;
    float changeStep;
};

class DisplayPages {
public:
    DisplayPages(MeasurementRegistry& registry, DisplayManager& manager);
    bool addMeasurementPage(LcdDisplay& display, uint8_t pageIndex, const MeasurementField* fields, uint8_t count);
    void service();
private:
    struct PageState {
        LcdDisplay* display = nullptr;
        uint8_t pageIndex = 0;
        const MeasurementField* fields = nullptr;
        uint8_t fieldCount = 0;
        int32_t lastValues[Config::MAX_LCD_ROWS] = {};
        MeasurementStatus lastStatuses[Config::MAX_LCD_ROWS] = {};
        bool initialized = false;
    };
    MeasurementRegistry& registry;
    DisplayManager& manager;
    PageState pages[Config::MAX_DISPLAYS * Config::MAX_PAGES_PER_DISPLAY];
    uint8_t pageCount = 0;
    bool formatMeasurementLine(char* buffer, size_t size, const MeasurementField& field);
    bool hasMeasurementChanged(PageState& page, uint8_t fieldIndex, const Measurement& measurement);
    void updateMeasurementPage(PageState& page);
};
#pragma once

#include <Arduino.h>
#include "Config.h"
#include "Channel.h"
#include "DisplayManager.h"

// Единицы только для ОТОБРАЖЕНИЯ. Внутри системы давление всегда в атм.
// COUNT всегда последним (не единица, нужен для циклического переключения).
enum class PressureDisplayUnit : uint8_t {
    ATM,
    BAR,
    MPA,
    COUNT
};

class DisplayPages {
public:
    explicit DisplayPages(DisplayManager& manager);

    bool addTemplatePage(
        LcdDisplay& display,
        uint8_t pageIndex,
        const char* const* lines,
        uint8_t lineCount
    );

    bool bind(
        LcdDisplay& display,
        uint8_t pageIndex,
        uint8_t lineIndex,
        Channel& channel
    );

    void setPressureUnit(PressureDisplayUnit unit);
    PressureDisplayUnit getPressureUnit() const;
    void nextPressureUnit();

    // "значение единица" в текущих единицах отображения (для страницы аварии)
    void formatValueWithUnit(
        const Channel& channel,
        float value,
        char* output,
        size_t outputSize
    ) const;

    bool isChannelBoundToDisplay(const LcdDisplay& display, const Channel& channel) const;
    void service();

private:
    static constexpr uint8_t MAX_BINDINGS_PER_LINE = Config::MAX_LCD_COLS;
    static constexpr uint8_t MAX_PAGES =
        Config::MAX_DISPLAYS * Config::MAX_PAGES_PER_DISPLAY;

    struct PageState {
        LcdDisplay* display = nullptr;
        uint8_t pageIndex = 0;
        uint8_t lineCount = 0;

        const char* lines[Config::MAX_LCD_ROWS] = {};

        Channel* bindings[Config::MAX_LCD_ROWS]
                         [MAX_BINDINGS_PER_LINE] = {};

        uint8_t bindingCounts[Config::MAX_LCD_ROWS] = {};

        char lastLines[Config::MAX_LCD_ROWS]
                      [Config::MAX_LCD_COLS + 1] = {};

        bool initialized = false;
    };

    DisplayManager& manager;
    PageState pages[MAX_PAGES] = {};
    uint8_t pageCount = 0;
    PressureDisplayUnit pressureUnit = PressureDisplayUnit::ATM;

    PageState* findPage(LcdDisplay& display, uint8_t pageIndex);

    bool renderLine(
        PageState& page,
        uint8_t lineIndex,
        char* output,
        size_t outputSize
    );

    bool appendText(
        char* output,
        size_t outputSize,
        size_t& used,
        const char* text
    );

    bool appendValue(
        char* output,
        size_t outputSize,
        size_t& used,
        const Channel& channel
    );

    bool appendUnit(
        char* output,
        size_t outputSize,
        size_t& used,
        const Channel& channel
    );

    bool appendChannelName(
        char* output,
        size_t outputSize,
        size_t& used,
        const Channel& channel
    );

    bool appendNumber(
        char* output,
        size_t outputSize,
        size_t& used,
        float value,
        uint8_t precision
    );

    bool isPressureChannel(const Channel& channel) const;

    // Единственное место правил отображения (используют и шаблоны, и страница аварии)
    float displayValue(const Channel& channel, float value) const;
    uint8_t displayPrecision(const Channel& channel) const;
    const char* displayUnit(const Channel& channel) const;

    float convertPressure(float value) const;
    uint8_t pressurePrecision() const;
    const char* pressureUnitName() const;
    void updatePage(PageState& page);
};
#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Config.h"

// =====================================================
// LCD DISPLAY
// =====================================================
class LcdDisplay {
public:
    LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows);
    void service();
    void printLine(uint8_t row, const char* text);
    void clear();
    bool isOnline() const;
    uint8_t columns() const;
    uint8_t rowCount() const;
private:
    uint8_t address;
    uint8_t cols;
    uint8_t rows;
    LiquidCrystal_I2C lcd;
    bool online;
    bool checked;
    unsigned long lastCheck;
    char cache[Config::MAX_LCD_ROWS][Config::MAX_LCD_COLS + 1];
    static uint8_t limitCols(uint8_t value);
    static uint8_t limitRows(uint8_t value);
    void resetCache();
};

// =====================================================
// SCREEN PAGE
// =====================================================
struct ScreenPage {
    char lines[Config::MAX_LCD_ROWS][Config::MAX_LCD_COLS + 1] = {};
    uint8_t lineCount = 0;
    bool setLine(uint8_t row, const char* text);
    void clear();
};

// =====================================================
// DISPLAY MANAGER
// =====================================================
class DisplayManager {
public:
    DisplayManager();
    bool addDisplay(LcdDisplay& display);
    bool addPage(LcdDisplay& display, const ScreenPage& page);
    bool replacePage(LcdDisplay& display, uint8_t pageIndex, const ScreenPage& page);
    bool removePage(LcdDisplay& display, uint8_t pageIndex);
    bool clearPages(LcdDisplay& display);
    bool showPage(LcdDisplay& display, uint8_t pageIndex);
    int8_t getPageCount(const LcdDisplay& display) const;
    void service();
    uint8_t displayCount() const;
private:
    struct DisplaySlot {
        LcdDisplay* display = nullptr;
        ScreenPage pages[Config::MAX_PAGES_PER_DISPLAY];
        uint8_t pageCount = 0;
        uint8_t currentPage = 0;
        bool blanked = false;
        bool dirty = true;
    };
    DisplaySlot displays[Config::MAX_DISPLAYS];
    uint8_t count;
    int8_t findDisplay(const LcdDisplay& display) const;
    static void renderPage(LcdDisplay& display, const ScreenPage& page);
};
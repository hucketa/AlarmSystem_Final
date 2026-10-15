#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>

namespace Config {
    constexpr uint8_t SDA_PIN = 8;
    constexpr uint8_t SCL_PIN = 9;
    constexpr uint8_t MAX_LCD_COLS = 20;
    constexpr uint8_t MAX_LCD_ROWS = 4;
    constexpr uint8_t MAX_DISPLAYS = 4;
    constexpr uint8_t MAX_PAGES_PER_DISPLAY = 8;
    constexpr unsigned long LCD_CHECK_INTERVAL = 1000;
    constexpr unsigned long DEFAULT_PAGE_INTERVAL = 3000;
}

class LcdDisplay {
public:
    LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows)
        : address(address), cols(limitCols(columns)), rows(limitRows(rows)),
          lcd(address, cols, rows), online(false), checked(false), lastCheck(0), cache{} {}

    void service() {
        unsigned long now = millis();
        if (checked && now - lastCheck < Config::LCD_CHECK_INTERVAL) return;
        checked = true;
        lastCheck = now;
        Wire.beginTransmission(address);
        bool connected = Wire.endTransmission() == 0;
        if (connected == online) return;
        online = connected;
        resetCache();
        if (!online) return;
        lcd.init();
        lcd.backlight();
        lcd.clear();
    }

    void printLine(uint8_t row, const char* text) {
        if (!online || row >= rows) return;
        char buffer[Config::MAX_LCD_COLS + 1] = {};
        size_t length = text ? strlen(text) : 0;
        if (length > cols) length = cols;
        if (length > 0) memcpy(buffer, text, length);
        for (size_t i = length; i < cols; i++) buffer[i] = ' ';
        buffer[cols] = '\0';
        if (strcmp(buffer, cache[row]) == 0) return;
        lcd.setCursor(0, row);
        lcd.print(buffer);
        memcpy(cache[row], buffer, cols + 1);
    }

    void clear() {
        if (online) lcd.clear();
        resetCache();
    }

    bool isOnline() const { return online; }
    uint8_t columns() const { return cols; }
    uint8_t rowCount() const { return rows; }

private:
    uint8_t address, cols, rows;
    LiquidCrystal_I2C lcd;
    bool online, checked;
    unsigned long lastCheck;
    char cache[Config::MAX_LCD_ROWS][Config::MAX_LCD_COLS + 1];

    static uint8_t limitCols(uint8_t value) {
        if (value < 1) return 1;
        if (value > Config::MAX_LCD_COLS) return Config::MAX_LCD_COLS;
        return value;
    }

    static uint8_t limitRows(uint8_t value) {
        if (value < 1) return 1;
        if (value > Config::MAX_LCD_ROWS) return Config::MAX_LCD_ROWS;
        return value;
    }

    void resetCache() {
        for (uint8_t row = 0; row < Config::MAX_LCD_ROWS; row++)
            cache[row][0] = '\0';
    }
};

struct ScreenPage {
    char lines[Config::MAX_LCD_ROWS][Config::MAX_LCD_COLS + 1] = {};
    uint8_t lineCount = 0;

    bool setLine(uint8_t row, const char* text) {
        if (row >= Config::MAX_LCD_ROWS) return false;
        size_t length = text ? strlen(text) : 0;
        if (length > Config::MAX_LCD_COLS) length = Config::MAX_LCD_COLS;
        if (length > 0) memcpy(lines[row], text, length);
        lines[row][length] = '\0';
        if (lineCount <= row) lineCount = row + 1;
        return true;
    }

    void clear() {
        for (uint8_t row = 0; row < Config::MAX_LCD_ROWS; row++)
            lines[row][0] = '\0';
        lineCount = 0;
    }
};

class DisplayManager {
public:
    DisplayManager() : count(0), displays{} {}

    bool addDisplay(LcdDisplay& display) {
        if (findDisplay(display) >= 0) return false;
        if (count >= Config::MAX_DISPLAYS) return false;
        DisplaySlot& slot = displays[count++];
        slot.display = &display;
        slot.pageCount = 0;
        slot.currentPage = 0;
        slot.pageInterval = Config::DEFAULT_PAGE_INTERVAL;
        slot.lastPageSwitch = millis();
        slot.blanked = false;
        return true;
    }

    bool addPage(LcdDisplay& display, const ScreenPage& page) {
        int8_t index = findDisplay(display);
        if (index < 0) return false;
        DisplaySlot& slot = displays[index];
        if (slot.pageCount >= Config::MAX_PAGES_PER_DISPLAY) return false;
        slot.pages[slot.pageCount++] = page;
        slot.currentPage = 0;
        slot.lastPageSwitch = millis();
        slot.blanked = false;
        return true;
    }

    bool clearPages(LcdDisplay& display) {
        int8_t index = findDisplay(display);
        if (index < 0) return false;
        DisplaySlot& slot = displays[index];
        slot.pageCount = 0;
        slot.currentPage = 0;
        slot.blanked = false;
        return true;
    }

    bool setPageInterval(LcdDisplay& display, unsigned long interval) {
        int8_t index = findDisplay(display);
        if (index < 0 || interval == 0) return false;
        DisplaySlot& slot = displays[index];
        slot.pageInterval = interval;
        slot.lastPageSwitch = millis();
        return true;
    }

    void service() {
        for (uint8_t i = 0; i < count; i++) {
            DisplaySlot& slot = displays[i];
            slot.display->service();
            if (slot.pageCount == 0) {
                if (!slot.blanked) slot.display->clear();
                slot.blanked = true;
                continue;
            }
            slot.blanked = false;
            unsigned long now = millis();
            if (slot.pageCount > 1 && now - slot.lastPageSwitch >= slot.pageInterval) {
                slot.currentPage++;
                if (slot.currentPage >= slot.pageCount) slot.currentPage = 0;
                slot.lastPageSwitch = now;
            }
            renderPage(*slot.display, slot.pages[slot.currentPage]);
        }
    }

    uint8_t displayCount() const { return count; }

private:
    struct DisplaySlot {
        LcdDisplay* display = nullptr;
        ScreenPage pages[Config::MAX_PAGES_PER_DISPLAY];
        uint8_t pageCount = 0;
        uint8_t currentPage = 0;
        unsigned long pageInterval = Config::DEFAULT_PAGE_INTERVAL;
        unsigned long lastPageSwitch = 0;
        bool blanked = false;
    };

    DisplaySlot displays[Config::MAX_DISPLAYS];
    uint8_t count;

    int8_t findDisplay(const LcdDisplay& display) const {
        for (uint8_t i = 0; i < count; i++)
            if (displays[i].display == &display) return i;
        return -1;
    }

    static void renderPage(LcdDisplay& display, const ScreenPage& page) {
        for (uint8_t row = 0; row < display.rowCount(); row++) {
            if (row < page.lineCount) display.printLine(row, page.lines[row]);
            else display.printLine(row, "");
        }
    }
};

LcdDisplay temperatureDisplay(0x25, 16, 2);
LcdDisplay pressureDisplay(0x27, 16, 2);
DisplayManager displayManager;

void setup() {
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);

    displayManager.addDisplay(temperatureDisplay);
    displayManager.addDisplay(pressureDisplay);

    ScreenPage temperaturePage;
    temperaturePage.setLine(0, "TEMPERATURE");
    temperaturePage.setLine(1, "System ready");
    displayManager.addPage(temperatureDisplay, temperaturePage);

    ScreenPage pressurePage;
    pressurePage.setLine(0, "PRESSURE");
    pressurePage.setLine(1, "System ready");
    displayManager.addPage(pressureDisplay, pressurePage);
}

void loop() {
    displayManager.service();
}

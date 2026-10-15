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
    constexpr unsigned long LCD_CHECK_INTERVAL = 1000;
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

    void setLine(uint8_t row, const char* text) {
        if (row >= Config::MAX_LCD_ROWS) return;
        size_t length = text ? strlen(text) : 0;
        if (length > Config::MAX_LCD_COLS) length = Config::MAX_LCD_COLS;
        if (length > 0) memcpy(lines[row], text, length);
        lines[row][length] = '\0';
        if (lineCount <= row) lineCount = row + 1;
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
        displays[count].display = &display;
        displays[count].page.clear();
        count++;
        return true;
    }

    bool setPage(LcdDisplay& display, const ScreenPage& page) {
        int8_t index = findDisplay(display);
        if (index < 0) return false;
        displays[index].page = page;
        return true;
    }

    void service() {
        for (uint8_t i = 0; i < count; i++) {
            displays[i].display->service();
            renderPage(displays[i].display, displays[i].page);
        }
    }

    uint8_t displayCount() const { return count; }

private:
    struct DisplaySlot {
        LcdDisplay* display = nullptr;
        ScreenPage page;
    };

    DisplaySlot displays[Config::MAX_DISPLAYS];
    uint8_t count;

    int8_t findDisplay(const LcdDisplay& display) const {
        for (uint8_t i = 0; i < count; i++)
            if (displays[i].display == &display) return i;
        return -1;
    }

    static void renderPage(LcdDisplay* display, const ScreenPage& page) {
        for (uint8_t row = 0; row < display->rowCount(); row++) {
            if (row < page.lineCount) display->printLine(row, page.lines[row]);
            else display->printLine(row, "");
        }
    }
};

LcdDisplay temperatureDisplay(0x25, 16, 2);
LcdDisplay pressureDisplay(0x27, 16, 2);
DisplayManager displayManager;

void setup() {
    Serial.begin(115200);
    Wire.begin(Config::SDA_PIN, Config::SCL_PIN);
    displayManager.addDisplay(temperatureDisplay);
    displayManager.addDisplay(pressureDisplay);
}

void loop() {
    displayManager.service();
}
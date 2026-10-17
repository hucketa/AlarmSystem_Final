
#include "DisplayManager.h"
#include <Wire.h>
#include <string.h>

// =====================================================
// LcdDisplay
// =====================================================
LcdDisplay::LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows) : address(address), cols(limitCols(columns)), rows(limitRows(rows)), lcd(address, cols, rows), online(false), checked(false), lastCheck(0), cache{} {
}

// =====================================================
// CHECK DISPLAY CONNECTION
// =====================================================
void LcdDisplay::service() {
    unsigned long now = millis();
    if (checked && now - lastCheck < Config::LCD_CHECK_INTERVAL) {
        return;
    }
    checked = true;
    lastCheck = now;
    Wire.beginTransmission(address);
    bool connected = Wire.endTransmission() == 0;
    if (connected == online) {
        return;
    }
    online = connected;
    resetCache();
    if (!online) {
        return;
    }
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

// =====================================================
// PRINT LINE
// =====================================================
void LcdDisplay::printLine(uint8_t row, const char* text) {
    if (!online || row >= rows) {
        return;
    }
    char buffer[Config::MAX_LCD_COLS + 1] = {};
    size_t length = text ? strlen(text) : 0;
    if (length > cols) {
        length = cols;
    }
    if (length > 0) {
        memcpy(buffer, text, length);
    }
    for (size_t i = length; i < cols; i++) {
        buffer[i] = ' ';
    }
    buffer[cols] = '\0';
    if (strcmp(buffer, cache[row]) == 0) {
        return;
    }
    lcd.setCursor(0, row);
    lcd.print(buffer);
    memcpy(cache[row], buffer, cols + 1);
}

// =====================================================
// CLEAR DISPLAY
// =====================================================
void LcdDisplay::clear() {
    if (online) {
        lcd.clear();
    }
    resetCache();
}

// =====================================================
// DISPLAY STATUS
// =====================================================
bool LcdDisplay::isOnline() const {
    return online;
}

// =====================================================
// DISPLAY WIDTH
// =====================================================
uint8_t LcdDisplay::columns() const {
    return cols;
}

// =====================================================
// DISPLAY HEIGHT
// =====================================================
uint8_t LcdDisplay::rowCount() const {
    return rows;
}

// =====================================================
// LIMIT DISPLAY WIDTH
// =====================================================
uint8_t LcdDisplay::limitCols(uint8_t value) {
    if (value < 1) {
        return 1;
    }
    if (value > Config::MAX_LCD_COLS) {
        return Config::MAX_LCD_COLS;
    }
    return value;
}

// =====================================================
// LIMIT DISPLAY HEIGHT
// =====================================================
uint8_t LcdDisplay::limitRows(uint8_t value) {
    if (value < 1) {
        return 1;
    }
    if (value > Config::MAX_LCD_ROWS) {
        return Config::MAX_LCD_ROWS;
    }
    return value;
}

// =====================================================
// RESET DISPLAY CACHE
// =====================================================
void LcdDisplay::resetCache() {
    for (uint8_t row = 0; row < Config::MAX_LCD_ROWS; row++) {
        cache[row][0] = '\0';
    }
}

// =====================================================
// ScreenPage
// =====================================================
bool ScreenPage::setLine(uint8_t row, const char* text) {
    if (row >= Config::MAX_LCD_ROWS) {
        return false;
    }
    size_t length = text ? strlen(text) : 0;
    if (length > Config::MAX_LCD_COLS) {
        length = Config::MAX_LCD_COLS;
    }
    if (length > 0) {
        memcpy(lines[row], text, length);
    }
    lines[row][length] = '\0';
    if (lineCount <= row) {
        lineCount = row + 1;
    }
    return true;
}

// =====================================================
// CLEAR PAGE
// =====================================================
void ScreenPage::clear() {
    for (uint8_t row = 0; row < Config::MAX_LCD_ROWS; row++) {
        lines[row][0] = '\0';
    }
    lineCount = 0;
}

// =====================================================
// DisplayManager
// =====================================================
DisplayManager::DisplayManager() : displays{}, count(0) {
}

// =====================================================
// REGISTER DISPLAY
// =====================================================
bool DisplayManager::addDisplay(LcdDisplay& display) {
    if (findDisplay(display) >= 0) {
        return false;
    }
    if (count >= Config::MAX_DISPLAYS) {
        return false;
    }
    DisplaySlot& slot = displays[count++];
    slot.display = &display;
    slot.pageCount = 0;
    slot.currentPage = 0;
    slot.blanked = false;
    return true;
}

// =====================================================
// ADD PAGE
// =====================================================
bool DisplayManager::addPage(LcdDisplay& display, const ScreenPage& page) {
    int8_t index = findDisplay(display);
    if (index < 0) {
        return false;
    }
    DisplaySlot& slot = displays[index];
    if (slot.pageCount >= Config::MAX_PAGES_PER_DISPLAY) {
        return false;
    }
    if (slot.pageCount == 0) {
        slot.currentPage = 0;
    }
    slot.pages[slot.pageCount++] = page;
    slot.blanked = false;
    return true;
}

// =====================================================
// REPLACE PAGE
// =====================================================
bool DisplayManager::replacePage(LcdDisplay& display, uint8_t pageIndex, const ScreenPage& page) {
    int8_t index = findDisplay(display);
    if (index < 0) {
        return false;
    }
    DisplaySlot& slot = displays[index];
    if (pageIndex >= slot.pageCount) {
        return false;
    }
    slot.pages[pageIndex] = page;
    return true;
}

// =====================================================
// REMOVE PAGE
// =====================================================
bool DisplayManager::removePage(LcdDisplay& display, uint8_t pageIndex) {
    int8_t index = findDisplay(display);
    if (index < 0) {
        return false;
    }
    DisplaySlot& slot = displays[index];
    if (pageIndex >= slot.pageCount) {
        return false;
    }
    uint8_t oldCurrent = slot.currentPage;
    for (uint8_t i = pageIndex; i + 1 < slot.pageCount; i++) {
        slot.pages[i] = slot.pages[i + 1];
    }
    slot.pageCount--;
    if (slot.pageCount == 0) {
        slot.currentPage = 0;
    } else if (pageIndex < oldCurrent) {
        slot.currentPage = oldCurrent - 1;
    } else if (oldCurrent >= slot.pageCount) {
        slot.currentPage = 0;
    } else {
        slot.currentPage = oldCurrent;
    }
    slot.blanked = false;
    return true;
}

// =====================================================
// CLEAR DISPLAY PAGES
// =====================================================
bool DisplayManager::clearPages(LcdDisplay& display) {
    int8_t index = findDisplay(display);
    if (index < 0) {
        return false;
    }
    DisplaySlot& slot = displays[index];
    slot.pageCount = 0;
    slot.currentPage = 0;
    slot.blanked = false;
    return true;
}

// =====================================================
// SHOW PAGE
// =====================================================
bool DisplayManager::showPage(LcdDisplay& display, uint8_t pageIndex) {
    int8_t index = findDisplay(display);
    if (index < 0) {
        return false;
    }
    DisplaySlot& slot = displays[index];
    if (pageIndex >= slot.pageCount) {
        return false;
    }
    slot.currentPage = pageIndex;
    slot.blanked = false;
    return true;
}

// =====================================================
// GET PAGE COUNT
// =====================================================
int8_t DisplayManager::getPageCount(const LcdDisplay& display) const {
    int8_t index = findDisplay(display);
    if (index < 0) {
        return -1;
    }
    return displays[index].pageCount;
}

// =====================================================
// UPDATE DISPLAYS
// =====================================================
void DisplayManager::service() {
    for (uint8_t i = 0; i < count; i++) {
        DisplaySlot& slot = displays[i];
        slot.display->service();
        if (slot.pageCount == 0) {
            if (!slot.blanked) {
                slot.display->clear();
            }
            slot.blanked = true;
            continue;
        }
        slot.blanked = false;
        renderPage(*slot.display, slot.pages[slot.currentPage]);
    }
}

// =====================================================
// DISPLAY COUNT
// =====================================================
uint8_t DisplayManager::displayCount() const {
    return count;
}

// =====================================================
// FIND DISPLAY
// =====================================================
int8_t DisplayManager::findDisplay(const LcdDisplay& display) const {
    for (uint8_t i = 0; i < count; i++) {
        if (displays[i].display == &display) {
            return i;
        }
    }
    return -1;
}

// =====================================================
// RENDER PAGE
// =====================================================
void DisplayManager::renderPage(LcdDisplay& display, const ScreenPage& page) {
    for (uint8_t row = 0; row < display.rowCount(); row++) {
        if (row < page.lineCount) {
            display.printLine(row, page.lines[row]);
        } else {
            display.printLine(row, "");
        }
    }
}

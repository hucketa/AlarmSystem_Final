#include "DisplayPages.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

DisplayPages::DisplayPages(DisplayManager& manager) : manager(manager) {
}

bool DisplayPages::addTemplatePage(LcdDisplay& display, uint8_t pageIndex, const char* const* lines, uint8_t lineCount) {
    int8_t total = manager.getPageCount(display);
    if (total < 0 || pageIndex >= total || lines == nullptr || lineCount == 0 || lineCount > display.rowCount() || lineCount > Config::MAX_LCD_ROWS || pageCount >= MAX_PAGES) return false;
    if (findPage(display, pageIndex) != nullptr) return false;
    for (uint8_t i = 0; i < lineCount; i++) {
        if (lines[i] == nullptr) return false;
    }
    PageState& page = pages[pageCount++];
    page.display = &display;
    page.pageIndex = pageIndex;
    page.lineCount = lineCount;
    for (uint8_t i = 0; i < lineCount; i++) page.lines[i] = lines[i];
    page.initialized = false;
    return true;
}

bool DisplayPages::bind(LcdDisplay& display, uint8_t pageIndex, uint8_t lineIndex, Channel& channel) {
    PageState* page = findPage(display, pageIndex);
    if (page == nullptr || lineIndex >= page->lineCount || page->bindingCounts[lineIndex] >= MAX_BINDINGS_PER_LINE) return false;
    uint8_t index = page->bindingCounts[lineIndex]++;
    page->bindings[lineIndex][index] = &channel;
    page->initialized = false;
    return true;
}

void DisplayPages::setPressureUnit(PressureDisplayUnit unit) {
    if (pressureUnit == unit) return;
    pressureUnit = unit;
    for (uint8_t i = 0; i < pageCount; i++) pages[i].initialized = false;
}

PressureDisplayUnit DisplayPages::getPressureUnit() const {
    return pressureUnit;
}

void DisplayPages::nextPressureUnit() {
    const uint8_t next = (static_cast<uint8_t>(pressureUnit) + 1) % static_cast<uint8_t>(PressureDisplayUnit::COUNT);
    setPressureUnit(static_cast<PressureDisplayUnit>(next));
}

void DisplayPages::service() {
    for (uint8_t i = 0; i < pageCount; i++) updatePage(pages[i]);
}

DisplayPages::PageState* DisplayPages::findPage(LcdDisplay& display, uint8_t pageIndex) {
    for (uint8_t i = 0; i < pageCount; i++) {
        if (pages[i].display == &display && pages[i].pageIndex == pageIndex) return &pages[i];
    }
    return nullptr;
}

bool DisplayPages::appendText(char* output, size_t outputSize, size_t& used, const char* text) {
    if (text == nullptr || used >= outputSize) return false;
    size_t length = strlen(text);
    if (length >= outputSize - used) length = outputSize - used - 1;
    memcpy(output + used, text, length);
    used += length;
    output[used] = '\0';
    return true;
}

bool DisplayPages::appendNumber(char* output, size_t outputSize, size_t& used, float value, uint8_t precision) {
    if (!isfinite(value)) return appendText(output, outputSize, used, "---");
    char number[24];
    snprintf(number, sizeof(number), "%.*f", precision, value);
    return appendText(output, outputSize, used, number);
}

// =====================================================
// ПРАВИЛА ОТОБРАЖЕНИЯ (единое место)
// =====================================================
bool DisplayPages::isPressureChannel(const Channel& channel) const {
    return channel.getUnit() != nullptr && strcmp(channel.getUnit(), "atm") == 0;
}

float DisplayPages::convertPressure(float value) const {
    switch (pressureUnit) {
        case PressureDisplayUnit::BAR:
            return value * 1.01325f;
        case PressureDisplayUnit::MPA:
            return value * 0.101325f;
        case PressureDisplayUnit::ATM:
        default:
            return value;
    }
}

uint8_t DisplayPages::pressurePrecision() const {
    switch (pressureUnit) {
        case PressureDisplayUnit::BAR:
            return Config::PRESSURE_DECIMALS_BAR;
        case PressureDisplayUnit::MPA:
            return Config::PRESSURE_DECIMALS_MPA;
        case PressureDisplayUnit::ATM:
        default:
            return Config::PRESSURE_DECIMALS_ATM;
    }
}

const char* DisplayPages::pressureUnitName() const {
    switch (pressureUnit) {
        case PressureDisplayUnit::BAR:
            return "bar";
        case PressureDisplayUnit::MPA:
            return "MPa";
        case PressureDisplayUnit::ATM:
        default:
            return "ATM";
    }
}

float DisplayPages::displayValue(const Channel& channel, float value) const {
    return isPressureChannel(channel) ? convertPressure(value) : value;
}

uint8_t DisplayPages::displayPrecision(const Channel& channel) const {
    return isPressureChannel(channel) ? pressurePrecision() : channel.getPrecision();
}

const char* DisplayPages::displayUnit(const Channel& channel) const {
    if (isPressureChannel(channel)) return pressureUnitName();
    const char* unit = channel.getUnit();
    if (unit == nullptr) return "";
    if (strcmp(unit, "C") == 0) return "\xDF" "C";
    return unit;
}

void DisplayPages::formatValueWithUnit(const Channel& channel, float value, char* output, size_t outputSize) const {
    snprintf(output, outputSize, "%.*f %s", displayPrecision(channel), displayValue(channel, value), displayUnit(channel));
}

// =====================================================
// ТОКЕНЫ ШАБЛОНА
// =====================================================
bool DisplayPages::appendChannelName(char* output, size_t outputSize, size_t& used, const Channel& channel) {
    return appendText(output, outputSize, used, channel.getName());
}

bool DisplayPages::appendValue(char* output, size_t outputSize, size_t& used, const Channel& channel) {
    if (channel.getStatus() == MeasurementStatus::NOT_INITIALIZED) return appendText(output, outputSize, used, "---");
    if (channel.getStatus() == MeasurementStatus::ERROR) return appendText(output, outputSize, used, "POMYLKA");
    return appendNumber(output, outputSize, used, displayValue(channel, channel.getValue()), displayPrecision(channel));
}

bool DisplayPages::appendUnit(char* output, size_t outputSize, size_t& used, const Channel& channel) {
    if (channel.getStatus() != MeasurementStatus::VALID) return true;
    return appendText(output, outputSize, used, displayUnit(channel));
}

bool DisplayPages::renderLine(PageState& page, uint8_t lineIndex, char* output, size_t outputSize) {
    output[0] = '\0';
    const char* source = page.lines[lineIndex];
    size_t used = 0;
    uint8_t channelIndex = 0;
    bool channelNameSeen = false;
    while (*source != '\0' && used < outputSize - 1) {
        if (*source != '{') {
            char character[2] = {*source++, '\0'};
            if (!appendText(output, outputSize, used, character)) return false;
            continue;
        }
        const char* end = strchr(source, '}');
        if (end == nullptr) return appendText(output, outputSize, used, source);
        size_t tokenLength = end - source - 1;
        char token[24];
        if (tokenLength >= sizeof(token)) return false;
        memcpy(token, source + 1, tokenLength);
        token[tokenLength] = '\0';
        if (strcmp(token, "CHANEL_NAME") == 0 || strcmp(token, "CHANNEL_NAME") == 0) {
            if (channelNameSeen) channelIndex++;
            channelNameSeen = true;
        }
        if (channelIndex >= page.bindingCounts[lineIndex]) return false;
        Channel& channel = *page.bindings[lineIndex][channelIndex];
        bool result = true;
        if (strcmp(token, "CHANEL_NAME") == 0 || strcmp(token, "CHANNEL_NAME") == 0) {
            result = appendChannelName(output, outputSize, used, channel);
        } else if (strcmp(token, "VALUE") == 0) {
            result = appendValue(output, outputSize, used, channel);
        } else if (strcmp(token, "UNIT") == 0) {
            result = appendUnit(output, outputSize, used, channel);
        } else {
            result = appendText(output, outputSize, used, "{");
            result = result && appendText(output, outputSize, used, token);
            result = result && appendText(output, outputSize, used, "}");
        }
        if (!result) return false;
        source = end + 1;
    }
    output[used] = '\0';
    return true;
}

void DisplayPages::updatePage(PageState& page) {
    ScreenPage screenPage;
    bool changed = !page.initialized;
    for (uint8_t row = 0; row < page.lineCount; row++) {
        char line[Config::MAX_LCD_COLS + 1];
        if (!renderLine(page, row, line, sizeof(line))) snprintf(line, sizeof(line), "POMYLKA SHABLONU");
        screenPage.setLine(row, line);
        if (strcmp(line, page.lastLines[row]) != 0) changed = true;
        snprintf(page.lastLines[row], sizeof(page.lastLines[row]), "%s", line);
    }
    if (changed) {
        if (manager.replacePage(*page.display, page.pageIndex, screenPage)) page.initialized = true;
    }
}

bool DisplayPages::isChannelBoundToDisplay(const LcdDisplay& display, const Channel& channel) const {
    for (uint8_t i = 0; i < pageCount; i++) {
        const PageState& page = pages[i];
        if (page.display != &display) continue;
        for (uint8_t row = 0; row < page.lineCount; row++) {
            for (uint8_t j = 0; j < page.bindingCounts[row]; j++) {
                if (page.bindings[row][j] == &channel) return true;
            }
        }
    }
    return false;
}
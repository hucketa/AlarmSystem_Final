#include "DisplayPages.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <limits.h>

// =====================================================
// CONSTRUCTOR
// =====================================================
DisplayPages::DisplayPages(MeasurementRegistry& registry, DisplayManager& manager) : registry(registry), manager(manager), pages{} {
}

// =====================================================
// REGISTER MEASUREMENT PAGE
// =====================================================
bool DisplayPages::addMeasurementPage(LcdDisplay& display, uint8_t pageIndex, const MeasurementField* fields, uint8_t count) {
    int8_t pageTotal = manager.getPageCount(display);
    if (pageTotal < 0 || pageIndex >= pageTotal || fields == nullptr || count == 0 || count > display.rowCount() || count > Config::MAX_LCD_ROWS || pageCount >= Config::MAX_DISPLAYS * Config::MAX_PAGES_PER_DISPLAY) {
        return false;
    }
    for (uint8_t i = 0; i < count; i++) {
        if (fields[i].label == nullptr || fields[i].unit == nullptr || fields[i].changeStep <= 0.0f) {
            return false;
        }
    }
    PageState& page = pages[pageCount++];
    page.display = &display;
    page.pageIndex = pageIndex;
    page.fields = fields;
    page.fieldCount = count;
    page.initialized = false;
    return true;
}

// =====================================================
// SERVICE MEASUREMENT PAGES
// =====================================================
void DisplayPages::service() {
    for (uint8_t i = 0; i < pageCount; i++) {
        updateMeasurementPage(pages[i]);
    }
}

// =====================================================
// FORMAT MEASUREMENT LINE
// =====================================================
bool DisplayPages::formatMeasurementLine(char* buffer, size_t size, const MeasurementField& field) {
    Measurement measurement;
    if (!registry.get(field.channel, measurement) || measurement.status == MeasurementStatus::NOT_INITIALIZED) {
        snprintf(buffer, size, "%s: ---", field.label);
        return true;
    }
    if (measurement.status == MeasurementStatus::ERROR) {
        snprintf(buffer, size, "%s: ERROR", field.label);
        return true;
    }
    if (strcmp(field.unit, "C") == 0) {
        snprintf(buffer, size, "%s: %.*f%cC", field.label, field.precision, measurement.value, (char)223);
    } else {
        snprintf(buffer, size, "%s: %.*f%s", field.label, field.precision, measurement.value, field.unit);
    }
    return true;
}

// =====================================================
// CHECK MEASUREMENT CHANGES
// =====================================================
bool DisplayPages::hasMeasurementChanged(PageState& page, uint8_t fieldIndex, const Measurement& measurement) {
    MeasurementStatus status = measurement.status;
    int32_t value = status == MeasurementStatus::VALID ? (int32_t)roundf(measurement.value / page.fields[fieldIndex].changeStep) : INT32_MIN;
    return !page.initialized || value != page.lastValues[fieldIndex] || status != page.lastStatuses[fieldIndex];
}

// =====================================================
// UPDATE MEASUREMENT PAGE
// =====================================================
void DisplayPages::updateMeasurementPage(PageState& page) {
    ScreenPage screenPage;
    Measurement measurements[Config::MAX_LCD_ROWS];
    int32_t values[Config::MAX_LCD_ROWS];
    MeasurementStatus statuses[Config::MAX_LCD_ROWS];
    bool changed = !page.initialized;
    for (uint8_t i = 0; i < page.fieldCount; i++) {
        if (!registry.get(page.fields[i].channel, measurements[i])) {
            measurements[i].value = 0.0f;
            measurements[i].status = MeasurementStatus::NOT_INITIALIZED;
        }
        statuses[i] = measurements[i].status;
        values[i] = statuses[i] == MeasurementStatus::VALID ? (int32_t)roundf(measurements[i].value / page.fields[i].changeStep) : INT32_MIN;
        if (hasMeasurementChanged(page, i, measurements[i])) {
            changed = true;
        }
        char line[Config::MAX_LCD_COLS + 1];
        formatMeasurementLine(line, sizeof(line), page.fields[i]);
        screenPage.setLine(i, line);
    }
    if (!changed || !manager.replacePage(*page.display, page.pageIndex, screenPage)) {
        return;
    }
    for (uint8_t i = 0; i < page.fieldCount; i++) {
        page.lastValues[i] = values[i];
        page.lastStatuses[i] = statuses[i];
    }
    page.initialized = true;
}
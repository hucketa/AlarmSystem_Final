#include "PressureSensor.h"
#include <math.h>

PressureSensor::PressureSensor(uint8_t pin, Channel& pressureChannel, Channel& rawChannel, Channel& millivoltsChannel, const Config::PressureCalibrationPoint* table, size_t tableSize)
    : pin(pin), pressureChannel(pressureChannel), rawChannel(rawChannel), millivoltsChannel(millivoltsChannel), table(table), tableSize(tableSize) {
}

bool PressureSensor::begin() {
    if (!validateTable()) {
        Serial.println("[ERROR] Pressure calibration table is invalid");
        pressureChannel.setError(millis());
        rawChannel.setError(millis());
        millivoltsChannel.setError(millis());
        return false;
    }
    pinMode(pin, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(pin, ADC_11db);
    rawSum = 0;
    millivoltsSum = 0;
    sampleCount = 0;
    nextSampleUs = micros();
    started = true;
    return true;
}

void PressureSensor::service() {
    if (!started) return;
    uint32_t now = micros();
    if (static_cast<int32_t>(now - nextSampleUs) < 0) return;
    nextSampleUs = now + Config::PRESSURE_SAMPLE_INTERVAL_US;
    rawSum += analogRead(pin);
    millivoltsSum += analogReadMilliVolts(pin);
    sampleCount++;
    if (sampleCount < Config::PRESSURE_SAMPLES) return;
    finishMeasurement(millis());
}

bool PressureSensor::validateTable() const {
    if (table == nullptr || tableSize < 2) return false;
    for (size_t i = 1; i < tableSize; i++) {
        if (table[i].raw <= table[i - 1].raw) return false;
        if (!isfinite(table[i].pressure)) return false;
        if (table[i].pressure <= table[i - 1].pressure) return false;
    }
    return isfinite(table[0].pressure);
}

void PressureSensor::finishMeasurement(uint32_t timestamp) {
    const float averageRaw = static_cast<float>(rawSum) / Config::PRESSURE_SAMPLES;
    const float averageMillivolts = static_cast<float>(millivoltsSum) / Config::PRESSURE_SAMPLES;
    const float calculatedPressure = rawToPressure(averageRaw);
    const float roundedPressure = roundf(calculatedPressure * 100.0f) / 100.0f;
    rawChannel.updateValue(averageRaw, timestamp);
    millivoltsChannel.updateValue(averageMillivolts, timestamp);
    pressureChannel.updateValue(roundedPressure, timestamp);
    rawSum = 0;
    millivoltsSum = 0;
    sampleCount = 0;
}

float PressureSensor::rawToPressure(float raw) const {
    if (raw <= table[0].raw) return 0.0f;
    if (raw >= table[tableSize - 1].raw) return table[tableSize - 1].pressure;
    for (size_t i = 0; i < tableSize - 1; i++) {
        if (raw <= table[i + 1].raw) {
            float p1 = table[i].pressure;
            float p2 = table[i + 1].pressure;
            float r1 = table[i].raw;
            float r2 = table[i + 1].raw;
            return p1 + (raw - r1) * (p2 - p1) / (r2 - r1);
        }
    }
    return 0.0f;
}
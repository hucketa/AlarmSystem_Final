#include "BuzzerManager.h"

extern AlarmManager alarmManager;

void BuzzerManager::begin() {
    pinMode(Config::BUZZER_PIN, OUTPUT);
    digitalWrite(Config::BUZZER_PIN, LOW);
    pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
    lastButtonState = digitalRead(Config::BUTTON_PIN);
    muted = false;
    clickCount = 0;
    lastClickAt = 0;
    triplePressDetected = false;
}

void BuzzerManager::service() {
    const uint32_t now = millis();
    if (alarmManager.activeAlarmCount() == 0) {
        serviceButton(now);
        muted = false;
        alarmManager.acknowledgeTriggers();
        digitalWrite(Config::BUZZER_PIN, LOW);
        return;
    }
    if (alarmManager.hasNewTrigger()) {
        muted = false;
        alarmManager.acknowledgeTriggers();
    }
    serviceButton(now);
    updateBuzzer();
}

void BuzzerManager::serviceButton(uint32_t now) {
    const bool reading = digitalRead(Config::BUTTON_PIN);
    if (reading == lastButtonState) return;
    lastButtonState = reading;
    if (reading != LOW) return;
    if (alarmManager.activeAlarmCount() > 0) muted = true;
    if (++clickCount >= 3) {
        clickCount = 0;
        triplePressDetected = true;
    }
}

bool BuzzerManager::consumeTriplePress() {
    if (!triplePressDetected) return false;
    triplePressDetected = false;
    return true;
}

void BuzzerManager::updateBuzzer() {
    digitalWrite(Config::BUZZER_PIN, muted ? LOW : HIGH);
}
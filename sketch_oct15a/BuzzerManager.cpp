#include "BuzzerManager.h"

extern AlarmManager alarmManager;

void BuzzerManager::begin() {
    pinMode(Config::BUZZER_PIN, OUTPUT);
    digitalWrite(Config::BUZZER_PIN, LOW);
    pinMode(Config::BUTTON_PIN, INPUT_PULLUP);
    buttonRawState = digitalRead(Config::BUTTON_PIN);
    buttonStableState = buttonRawState;
    buttonChangedAt = millis();
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
    if (clickCount > 0 && now - lastClickAt > MULTI_CLICK_INTERVAL_MS) clickCount = 0;
    if (reading != buttonRawState) {
        buttonRawState = reading;
        buttonChangedAt = now;
    }
    if (now - buttonChangedAt < Config::BUTTON_DEBOUNCE_MS) return;
    if (buttonStableState == buttonRawState) return;
    buttonStableState = buttonRawState;
    if (buttonStableState != LOW) return;
    if (alarmManager.activeAlarmCount() > 0) muted = true;
    if (clickCount == 0 || now - lastClickAt > MULTI_CLICK_INTERVAL_MS) clickCount = 1;
    else clickCount++;
    lastClickAt = now;
    if (clickCount >= 3) {
        triplePressDetected = true;
        clickCount = 0;
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
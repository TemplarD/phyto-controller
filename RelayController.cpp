// RelayController.cpp
#include "RelayController.h"
#include "Config.h"
#include "DebugLogger.h"

RelayController::RelayController(uint8_t pin) : relayPin(pin) {}

void RelayController::begin() {
    pinMode(relayPin, OUTPUT);
    turnOff(); // Начинаем с выключенного состояния
    DEBUG_LOG("✅ Контроллер реле инициализирован на пине " + String(relayPin));
}

void RelayController::turnOn() {
    if (!currentState) {
        digitalWrite(relayPin, HIGH);
        currentState = true;
        EVENT_LOG("💡 Реле ВКЛЮЧЕНО");
        DEBUG_LOG("🔌 Реле: ВКЛ");
    }
}

void RelayController::turnOff() {
    if (currentState) {
        digitalWrite(relayPin, LOW);
        currentState = false;
        EVENT_LOG("💡 Реле ВЫКЛЮЧЕНО");
        DEBUG_LOG("🔌 Реле: ВЫКЛ");
    }
}

void RelayController::toggle() {
    if (currentState) turnOff();
    else turnOn();
}

bool RelayController::getState() {
    return currentState;
}

String RelayController::getStateString() {
    return currentState ? "ON" : "OFF";
}
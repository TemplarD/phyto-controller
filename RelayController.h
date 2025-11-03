// RelayController.h
#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

class RelayController {
public:
    RelayController(uint8_t pin);
    void begin();
    void turnOn();
    void turnOff();
    void toggle();
    bool getState();
    String getStateString();

private:
    uint8_t relayPin;
    bool currentState = false;
};

#endif
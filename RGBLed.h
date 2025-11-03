// RGBLed.h
#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>
#include <FastLED.h>

class RGBLed {
public:
    void begin();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setStatus(bool relayState, bool autoMode, float lux, float threshold);
    void blinkError();
    void blinkSuccess();
    void off();

private:
    CRGB leds[1]; // Один светодиод
    unsigned long lastUpdate = 0;
};

#endif
// RGBLed.cpp
#include "RGBLed.h"
#include "Config.h"

void RGBLed::begin() {
    FastLED.addLeds<WS2812B, RGB_LED_PIN, GRB>(leds, 1);
    setColor(0, 0, 0); // Выключить
    Serial.println("✅ RGB LED инициализирован на пине " + String(RGB_LED_PIN));
}

void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
    leds[0] = CRGB(r, g, b);
    FastLED.show();
}

void RGBLed::setStatus(bool relayState, bool autoMode, float lux, float threshold) {
    if (millis() - lastUpdate < 500) return; // Обновляем не чаще 2 раз в секунду
    lastUpdate = millis();

    if (relayState) {
        // Реле ВКЛ - ЗЕЛЕНЫЙ
        setColor(0, 255, 0);
    } else if (autoMode) {
        // Авторежим, реле ВЫКЛ - СИНИЙ
        setColor(0, 0, 255);
    } else {
        // Ручной режим, реле ВЫКЛ - ФИОЛЕТОВЫЙ
        setColor(255, 0, 255);
    }
    
    // Мигаем красным если освещенность близка к порогу
    if (autoMode && abs(lux - threshold) < 100) {
        if ((millis() / 500) % 2 == 0) {
            setColor(255, 100, 0); // ОРАНЖЕВЫЙ
        }
    }
}

void RGBLed::blinkError() {
    for(int i = 0; i < 3; i++) {
        setColor(255, 0, 0); // Красный
        delay(200);
        setColor(0, 0, 0);   // Выкл
        delay(200);
    }
}

void RGBLed::blinkSuccess() {
    for(int i = 0; i < 3; i++) {
        setColor(0, 255, 0); // Зеленый
        delay(200);
        setColor(0, 0, 0);   // Выкл
        delay(200);
    }
}

void RGBLed::off() {
    setColor(0, 0, 0);
}
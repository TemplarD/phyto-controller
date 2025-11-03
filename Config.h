// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === Пины ESP32 ===
const uint8_t RELAY_PIN = 4;
const uint8_t I2C_SDA = 13;
const uint8_t I2C_SCL = 14;
const uint8_t STATUS_LED = 2;
const uint8_t RGB_LED_PIN = 5;

// === Настройки по умолчанию ===
struct Settings {
    float lightThreshold = 500.0;
    uint32_t checkInterval = 10000;
    uint32_t sensorLogInterval = 5000;
    bool autoMode = true;
    bool manualOn = false;
    bool debugEnabled = true;
    uint32_t maxLogSize = 50 * 1024; // 50KB - ДОБАВЛЯЕМ
    String schedule = "08:00-20:00";
    // УБИРАЕМ wifiSSID и wifiPassword отсюда
};

extern Settings config;

void loadConfig();
void saveConfig();
void printConfig();

#endif

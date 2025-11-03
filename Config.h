// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === Пины ESP32 ===
const uint8_t RELAY_PIN = 4;
const uint8_t I2C_SDA = 13; //21
const uint8_t I2C_SCL = 14; //22
const uint8_t STATUS_LED = 2;        // Обычный LED (если есть)
const uint8_t RGB_LED_PIN = 5;       // Пин для WS2812B (можно изменить на 13, 14, 15, etc)

// === Настройки по умолчанию ===
struct Settings {
    float lightThreshold = 500.0;      // Порог освещенности (люкс)
    uint32_t checkInterval = 10000;    // Интервал проверки (мс)
    uint32_t sensorLogInterval = 5000; // Интервал логирования датчика (мс)
    bool autoMode = true;              // Автоматический режим
    bool manualOn = false;             // Ручное управление
    bool debugEnabled = true;          // Включить отладку
    String wifiSSID = "";
    String wifiPassword = "";
    String schedule = "08:00-20:00";   // Расписание
};

// Глобальный экземпляр настроек
extern Settings config;

// Объявления функций
void loadConfig();
void saveConfig();
void printConfig();

#endif

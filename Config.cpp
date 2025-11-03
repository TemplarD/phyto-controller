// Config.cpp
#include "Config.h"
#include "DebugLogger.h"

Settings config;

void loadConfig() {
    // Временная заглушка - позже реализуем EEPROM
    Serial.println("⚙️ Загружены настройки по умолчанию");
}

void saveConfig() {
    // Временная заглушка
    Serial.println("⚙️ Настройки сохранены (заглушка)");
}

void printConfig() {
    Serial.println("=== КОНФИГУРАЦИЯ СИСТЕМЫ ===");
    Serial.println("Пин реле: " + String(RELAY_PIN));
    Serial.println("I2C SDA: " + String(I2C_SDA));
    Serial.println("I2C SCL: " + String(I2C_SCL));
    Serial.println("Порог освещенности: " + String(config.lightThreshold));
    Serial.println("Интервал проверки: " + String(config.checkInterval));
    Serial.println("Режим: " + String(config.autoMode ? "Авто" : "Ручной"));
    Serial.println("Отладка: " + String(config.debugEnabled ? "ВКЛ" : "ВЫКЛ"));
    Serial.println("=================================");
}
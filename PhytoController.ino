// PhytoController.ino
#include "Config.h"
#include "DebugLogger.h"
#include "LightSensor.h"
#include "RelayController.h"
#include "RGBLed.h"  // Добавляем новый модуль

// Глобальные объекты
LightSensor lightSensor;
RelayController relayController(RELAY_PIN);
RGBLed rgbLed;  // Добавляем RGB светодиод

// Таймеры
unsigned long lastSensorCheck = 0;
unsigned long lastSensorLog = 0;
unsigned long lastBlink = 0;
unsigned long lastRGBUpdate = 0;
bool ledState = false;

// Объявление функций
void checkLightAndControl();
void logSensorData();
void testSensorConnection();

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n🌱 PhytoController Starting...");
    Serial.println("=================================");
    Serial.println("✅ Код скомпилирован успешно!");
    Serial.println("📋 Версия: 1.2 - с RGB индикацией");
    Serial.println("=================================");
    
    // Инициализация LED пина для индикации
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    
    // Базовая инициализация
    Serial.println("⚙️ Инициализация настроек...");
    loadConfig();
    
    Serial.println("🌈 Инициализация RGB LED...");
    rgbLed.begin();
    
    Serial.println("🔍 Инициализация датчика света...");
    lightSensor.begin();
    
    Serial.println("🔌 Инициализация реле...");
    relayController.begin();
    
    // Тест подключения датчика
    testSensorConnection();
    
    // Сигнал успешной инициализации
    rgbLed.blinkSuccess();
    
    Serial.println("🚀 Все системы инициализированы!");
    Serial.println("💡 Статусный LED на пине: " + String(STATUS_LED));
    Serial.println("🌈 RGB LED на пине: " + String(RGB_LED_PIN));
    Serial.println("📡 Готов к работе...");
    Serial.println("=================================");
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Мигаем обычным LED для индикации работы
    if (currentMillis - lastBlink > 1000) {
        lastBlink = currentMillis;
        ledState = !ledState;
        digitalWrite(STATUS_LED, ledState);
        Serial.println("💡 LED: " + String(ledState ? "ON" : "OFF") + " | Uptime: " + String(millis()/1000) + "s");
    }
    
    // Обновляем RGB индикатор
    if (currentMillis - lastRGBUpdate > 500) {
        lastRGBUpdate = currentMillis;
        float lux = lightSensor.getLux();
        rgbLed.setStatus(relayController.getState(), config.autoMode, lux, config.lightThreshold);
    }
    
    // Основная логика управления
    if (currentMillis - lastSensorCheck >= config.checkInterval) {
        lastSensorCheck = currentMillis;
        checkLightAndControl();
    }
    
    // Логирование показаний датчика
    if (currentMillis - lastSensorLog >= config.sensorLogInterval) {
        lastSensorLog = currentMillis;
        logSensorData();
    }
    
    delay(100);
}

void checkLightAndControl() {
    Serial.println("🔍 Проверка освещенности...");
    
    float lux = lightSensor.getLux();
    
    bool shouldBeOn = false;
    
    if (config.autoMode) {
        shouldBeOn = (lux < config.lightThreshold);
        Serial.println("🤖 Авторежим: " + String(shouldBeOn ? "ВКЛ" : "ВЫКЛ") + 
                     " | Lux: " + String(lux) + 
                     " | Порог: " + String(config.lightThreshold));
    } else {
        shouldBeOn = config.manualOn;
        Serial.println("👤 Ручной режим: " + String(shouldBeOn ? "ВКЛ" : "ВЫКЛ"));
    }
    
    // Применяем состояние
    if (shouldBeOn && !relayController.getState()) {
        relayController.turnOn();
    } else if (!shouldBeOn && relayController.getState()) {
        relayController.turnOff();
    }
}

void logSensorData() {
    float lux = lightSensor.getLux();
    if (lux >= 0) {
        Serial.println("📊 ДАННЫЕ: Lux=" + String(lux, 2) + 
                      " | Relay=" + (relayController.getState() ? "ON" : "OFF") +
                      " | Mode=" + (config.autoMode ? "AUTO" : "MANUAL"));
    }
}

// Функция тестирования подключения датчика
void testSensorConnection() {
    Serial.println("\n🔧 ТЕСТ ПОДКЛЮЧЕНИЯ ДАТЧИКА");
    Serial.println("============================");
    
    if (lightSensor.isAvailable()) {
        Serial.println("✅ Реальный датчик GY-30 (GY-30) подключен");
        float lux = lightSensor.getLux();
        Serial.println("📊 Текущая освещенность: " + String(lux, 2) + " lux");
    } else {
        Serial.println("⚠️ Реальный датчик не найден, используется симуляция");
        Serial.println("🔌 Проверь подключение:");
        Serial.println("   GY-30 3Vo -> ESP32 3.3V");
        Serial.println("   GY-30 GND -> ESP32 GND"); 
        Serial.println("   GY-30 SDA -> ESP32 GPIO13");
        Serial.println("   GY-30 SCL -> ESP32 GPIO14");
        Serial.println("🔧 Советы:");
        Serial.println("   - Проверь пайку контактов");
        Serial.println("   - Убедись в правильности подключения");
        Serial.println("   - Попробуй переподключить датчик");
    }
    Serial.println("============================\n");
    
    if (!lightSensor.isAvailable()) {
        rgbLed.blinkError(); // Мигаем красным при ошибке датчика
    }    
}

// PhytoController.ino - ИСПРАВЛЕННАЯ версия setup()
#include "Config.h"
#include "DebugLogger.h"
#include "LightSensor.h"
#include "RelayController.h"
#include "RGBLed.h"  
#include "WebAPI.h"  

// Глобальные объекты
LightSensor lightSensor;
RelayController relayController(RELAY_PIN);
RGBLed rgbLed;

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
    Serial.println("📋 Версия: 1.3 - с Web API и ротацией логов");
    Serial.println("=================================");
    
    // 1. СНАЧАЛА базовые пины
    Serial.println("🔧 Инициализация базовых пинов...");
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);
    
    // 2. Загрузка конфигурации
    Serial.println("⚙️ Загрузка настроек...");
    loadConfig();
    printConfig();
    
    // 3. Инициализация файловой системы и логирования (ПЕРВОЙ!)
    Serial.println("📝 Инициализация системы логирования...");
    DebugLogger::begin();
    DebugLogger::setMaxLogSize(config.maxLogSize);
    SYSTEM_LOG("🚀 Система запускается...");
    
    // 4. Инициализация RGB индикации
    Serial.println("🌈 Инициализация RGB LED...");
    rgbLed.begin();
    SYSTEM_LOG("✅ RGB LED инициализирован");
    
    // 5. Инициализация датчика света
    Serial.println("🔍 Инициализация датчика света GY-30...");
    if (lightSensor.begin()) {
        SYSTEM_LOG("✅ Датчик освещенности инициализирован");
    } else {
        SYSTEM_LOG("⚠️ Датчик не найден, режим симуляции");
    }
    
    // 6. Инициализация реле
    Serial.println("🔌 Инициализация реле...");
    relayController.begin();
    SYSTEM_LOG("✅ Контроллер реле инициализирован");
    
    // 7. Тестирование подключения датчика
    Serial.println("🔧 Тестирование подключения...");
    testSensorConnection();
    
    // 8. Инициализация Web API и WiFi
    Serial.println("🌐 Инициализация Web API...");
    webAPI.begin();
    SYSTEM_LOG("✅ Web API инициализирован");
    
    // 9. Финальная проверка и сигнал готовности
    Serial.println("🔍 Финальная проверка систем...");
    
    // Проверяем ключевые компоненты
    bool systemsReady = true;
    
    if (!lightSensor.isAvailable()) {
        Serial.println("⚠️ Внимание: датчик в режиме симуляции");
        SYSTEM_LOG("⚠️ Режим симуляции датчика");
    }
    
    // Проверяем реле
    Serial.println("🔌 Тест реле...");
    relayController.turnOn();
    delay(500);
    relayController.turnOff();
    SYSTEM_LOG("✅ Тест реле выполнен");
    
    // 10. Сигнал успешной инициализации
    Serial.println("🎉 Все системы инициализированы!");
    Serial.println("=================================");
    Serial.println("💡 Статусный LED: GPIO " + String(STATUS_LED));
    Serial.println("🌈 RGB LED: GPIO " + String(RGB_LED_PIN));
    Serial.println("🔌 Реле: GPIO " + String(RELAY_PIN));
    Serial.println("📡 Датчик: SDA=" + String(I2C_SDA) + " SCL=" + String(I2C_SCL));
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("📶 WiFi: " + WiFi.localIP().toString());
    } else {
        Serial.println("📶 WiFi: Точка доступа - 192.168.4.1");
    }
    
    Serial.println("⏰ Uptime: 0 сек");
    Serial.println("=================================");
    
    rgbLed.blinkSuccess();
    SYSTEM_LOG("🎯 Система готова к работе");
    
    // Начальное состояние RGB
    float lux = lightSensor.getLux();
    rgbLed.setStatus(relayController.getState(), config.autoMode, lux, config.lightThreshold);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Мигаем обычным LED для индикации работы
    if (currentMillis - lastBlink > 1000) {
        lastBlink = currentMillis;
        ledState = !ledState;
        digitalWrite(STATUS_LED, ledState);
        
        // Логируем статус каждые 10 секунд чтобы не засорять логи
        static uint8_t statusCounter = 0;
        if (++statusCounter >= 10) {
            statusCounter = 0;
            DEBUG_LOG("💡 LED: " + String(ledState ? "ON" : "OFF") + 
                     " | Uptime: " + String(millis()/1000) + "s" +
                     " | Free RAM: " + String(esp_get_free_heap_size()) + " bytes");
        }
    }
    
    // Обновляем RGB индикатор (каждые 500мс)
    if (currentMillis - lastRGBUpdate > 500) {
        lastRGBUpdate = currentMillis;
        float lux = lightSensor.getLux();
        rgbLed.setStatus(relayController.getState(), config.autoMode, lux, config.lightThreshold);
    }
    
    // Основная логика управления (по интервалу из конфига)
    if (currentMillis - lastSensorCheck >= config.checkInterval) {
        lastSensorCheck = currentMillis;
        checkLightAndControl();
    }
    
    // Логирование показаний датчика (по интервалу из конфига)
    if (currentMillis - lastSensorLog >= config.sensorLogInterval) {
        lastSensorLog = currentMillis;
        logSensorData();
    }
    
    // Обрабатываем веб-запросы
    webAPI.handleClient();
    
    delay(100); // Основная задержка цикла
}

void checkLightAndControl() {
    DEBUG_LOG("🔍 Проверка освещенности...");
    
    float lux = lightSensor.getLux();
    
    bool shouldBeOn = false;
    
    if (config.autoMode) {
        shouldBeOn = (lux < config.lightThreshold);
        DEBUG_LOG("🤖 Авторежим: " + String(shouldBeOn ? "ВКЛ" : "ВЫКЛ") + 
                 " | Lux: " + String(lux, 2) + 
                 " | Порог: " + String(config.lightThreshold));
    } else {
        shouldBeOn = config.manualOn;
        DEBUG_LOG("👤 Ручной режим: " + String(shouldBeOn ? "ВКЛ" : "ВЫКЛ"));
    }
    
    // Применяем состояние
    if (shouldBeOn && !relayController.getState()) {
        relayController.turnOn();
        EVENT_LOG("💡 Реле ВКЛ (Освещенность: " + String(lux, 2) + " lux)");
    } else if (!shouldBeOn && relayController.getState()) {
        relayController.turnOff();
        EVENT_LOG("💡 Реле ВЫКЛ (Освещенность: " + String(lux, 2) + " lux)");
    }
}

void logSensorData() {
    float lux = lightSensor.getLux();
    if (lux >= 0) {
        DebugLogger::logSensor(lux, relayController.getState());
        
        // Дополнительная информация в debug
        if (config.debugEnabled) {
            DEBUG_LOG("📊 Данные: Lux=" + String(lux, 2) + 
                     " | Relay=" + (relayController.getState() ? "ON" : "OFF") +
                     " | Mode=" + (config.autoMode ? "AUTO" : "MANUAL") +
                     " | Threshold=" + String(config.lightThreshold));
        }
    }
}

// Функция тестирования подключения датчика
void testSensorConnection() {
    Serial.println("\n🔧 ТЕСТ ПОДКЛЮЧЕНИЯ ДАТЧИКА");
    Serial.println("============================");
    
    // Проверка напряжения
    Serial.println("🔌 Проверка напряжений:");
    Serial.println("   ESP32 3.3V -> GY-30 VCC");
    Serial.println("   ESP32 GND  -> GY-30 GND");
    Serial.println("   ESP32 GPIO13 -> GY-30 SDA");
    Serial.println("   ESP32 GPIO14 -> GY-30 SCL");
    Serial.println("");
    Serial.println("⚡ На SDA/SCL должно быть ~3.3V через подтяжку");
    Serial.println("");
    
    if (lightSensor.isAvailable()) {
        Serial.println("✅ Реальный датчик GY-30 подключен");
        float lux = lightSensor.getLux();
        Serial.println("📊 Текущая освещенность: " + String(lux, 2) + " lux");
    } else {
        Serial.println("❌ Реальный датчик не найден!");
        Serial.println("");
        Serial.println("🔧 РЕШЕНИЯ:");
        Serial.println("1. ✅ Добавь подтягивающие резисторы 4.7кОм:");
        Serial.println("   SDA → 3.3V через 4.7кОм");
        Serial.println("   SCL → 3.3V через 4.7кОм");
        Serial.println("");
        Serial.println("2. ✅ Проверь распиновку GY-30:");
        Serial.println("   VCC - 3.3V (НЕ 5V!)");
        Serial.println("   GND - GND");
        Serial.println("   SDA - GPIO13");  
        Serial.println("   SCL - GPIO14");
        Serial.println("   ADDR - GND (адрес 0x23)");
        Serial.println("");
        Serial.println("3. ✅ Проверь пайку и контакты");
        Serial.println("");
        Serial.println("🔧 Используется режим симуляции");
    }
    Serial.println("============================\n");
    
    if (!lightSensor.isAvailable()) {
        rgbLed.blinkError();
    }    
}

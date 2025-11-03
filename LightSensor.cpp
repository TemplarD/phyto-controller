#include "LightSensor.h"
#include "Config.h"
#include "DebugLogger.h"

bool LightSensor::begin() {
    Serial.println("🔧 Инициализация BH1750...");
    
    // Жесткий рестарт I2C
    Wire.end();
    delay(100);
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    delay(100);
    
    // Пробуем 3 раза
    for(int i = 0; i < 3; i++) {
        if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
            sensorFound = true;
            simulationMode = false;
            DEBUG_LOG("✅ BH1750 подключен");
            return true;
        }
        delay(500);
    }
    
    DEBUG_LOG("❌ BH1750 не найден, симуляция");
    sensorFound = false;
    simulationMode = true;
    return false;
}

float LightSensor::getLux() {
    if (simulationMode) {
        if (millis() - lastRead > 5000) {
            simulatedLux += random(-100, 100);
            if (simulatedLux < 0) simulatedLux = 100;
            if (simulatedLux > 2000) simulatedLux = 1500;
            lastRead = millis();
        }
        DEBUG_LOG("🔆 СИМУЛЯЦИЯ: " + String(simulatedLux, 2) + " lux");
        return simulatedLux;
    }
    
    if (!sensorFound) {
        // Пробуем переинициализировать I2C при потере датчика
        static unsigned long lastRetry = 0;
        if (millis() - lastRetry > 10000) { // Каждые 10 секунд
            lastRetry = millis();
            DEBUG_LOG("🔄 Попытка переподключения BH1750...");
            Wire.begin(I2C_SDA, I2C_SCL);
            delay(100);
            if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
                sensorFound = true;
                DEBUG_LOG("✅ BH1750 переподключен");
            }
        }
        return -1.0;
    }
    
    float lux = lightMeter.readLightLevel();
    if (lux >= 0) {
        DEBUG_LOG("🔆 BH1750: " + String(lux, 2) + " lux");
        return lux;
    } else {
        DEBUG_LOG("❌ Ошибка чтения BH1750");
        return -1.0;
    }
}

bool LightSensor::isAvailable() {
    return sensorFound;
}

String LightSensor::getSensorInfo() {
    if (simulationMode) return "BH1750 (СИМУЛЯЦИЯ)";
    if (!sensorFound) return "Датчик недоступен";
    return "BH1750 (GY-30) - Режим: HIGH_RES";
}

void LightSensor::setSimulationMode(bool simulate) {
    simulationMode = simulate;
    DEBUG_LOG("🔧 Режим симуляции: " + String(simulate ? "ВКЛ" : "ВЫКЛ"));
}

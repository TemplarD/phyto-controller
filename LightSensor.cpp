// LightSensor.cpp
#include "LightSensor.h"
#include "Config.h"
#include "DebugLogger.h"

bool LightSensor::begin() {
    Serial.println("🔧 Инициализация GY-30...");  
    // Полный сброс I2C
    Wire.end();
    delay(100);
    
    // Пробуем разные скорости I2C: 100kHz, 50kHz, 400kHz
    long speeds[] = {100000, 50000, 400000};
    byte addresses[] = {0x23, 0x5C}; // Оба возможных адреса BH1750
    
    for (long speed : speeds) {
        Serial.println("🔄 Попытка на скорости: " + String(speed) + " Hz");
        
        Wire.begin(I2C_SDA, I2C_SCL, speed);
        delay(100);
        
        // Сканирование всех адресов для диагностики
        Serial.print("📡 Сканирование шины: ");
        bool foundAny = false;
        for(byte addr = 1; addr < 127; addr++) {
            Wire.beginTransmission(addr);
            byte error = Wire.endTransmission();
            if (error == 0) {
                Serial.print("0x" + String(addr, HEX) + " ");
                foundAny = true;
            }
        }
        if (!foundAny) Serial.print("нет устройств");
        Serial.println();
        
        // Попытка подключения к BH1750
        for (byte addr : addresses) {
            Serial.print("  Адрес 0x" + String(addr, HEX) + ": ");
            
            Wire.beginTransmission(addr);
            byte error = Wire.endTransmission();
            
            if (error == 0) {
                Serial.print("отвечает -> ");
                
                if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, addr)) {
                    Serial.println("ПОДКЛЮЧЕН!");
                    sensorFound = true;
                    simulationMode = false;
                    
                    // Тестовое чтение
                    float testLux = lightMeter.readLightLevel();
                    Serial.println("✅ Тестовое чтение: " + String(testLux, 2) + " lux");
                    
                    return true;
                } else {
                    Serial.println("ошибка инициализации");
                }
            } else if (error == 2) {
                Serial.println("NACK ошибка");
            } else if (error == 4) {
                Serial.println("ошибка передачи");
            } else {
                Serial.println("код ошибки: " + String(error));
            }
        }
        
        Wire.end();
        delay(100);
    }
    
    // Если ничего не сработало - диагностика
    Serial.println("❌ GY-30 не найден после всех попыток");
    Serial.println("🔧 Диагностика:");
    Serial.println("   - Проверь питание 3.3V на датчике");
    Serial.println("   - Проверь подключение ADDR к GND"); 
    Serial.println("   - Проверь резисторы 4.7кОм на SDA/SCL к 3.3V");
    Serial.println("   - Проверь целостность всех проводов");
    Serial.println("   - Попробуй другие пины I2C");
    
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
        static unsigned long lastRetry = 0;
        if (millis() - lastRetry > 10000) {
            lastRetry = millis();
            DEBUG_LOG("🔄 Попытка переподключения GY-30...");
            Wire.begin(I2C_SDA, I2C_SCL);
            delay(100);
            if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
                sensorFound = true;
                DEBUG_LOG("✅ GY-30 переподключен");
            }
        }
        return -1.0;
    }
    
    float lux = lightMeter.readLightLevel();
    if (lux >= 0) {
        DEBUG_LOG("🔆 GY-30: " + String(lux, 2) + " lux");
        return lux;
    } else {
        DEBUG_LOG("❌ Ошибка чтения GY-30");
        return -1.0;
    }
}

bool LightSensor::isAvailable() {
    return sensorFound;
}

String LightSensor::getSensorInfo() {
    if (simulationMode) return "GY-30 (СИМУЛЯЦИЯ)";
    if (!sensorFound) return "Датчик недоступен";
    return "GY-30 - Режим: HIGH_RES";
}

void LightSensor::setSimulationMode(bool simulate) {
    simulationMode = simulate;
    DEBUG_LOG("🔧 Режим симуляции: " + String(simulate ? "ВКЛ" : "ВЫКЛ"));
}

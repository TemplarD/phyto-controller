// DebugLogger.cpp
#include "DebugLogger.h"
#include "Config.h"
#include <LittleFS.h>

uint32_t DebugLogger::maxLogSize = 1024 * 50; // 50KB по умолчанию

void DebugLogger::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("❌ Ошибка инициализации LittleFS");
        return;
    }
    
    // Создаем папку для логов если нужно
    LittleFS.mkdir("/logs");
    
    SYSTEM_LOG("🚀 Система логирования инициализирована");
    DEBUG_LOG("📁 Файловая система готова");
}

void DebugLogger::log(const String& message, LogType type) {
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    String timestamp = String(hours) + ":" + String(minutes % 60) + ":" + String(seconds % 60);
    
    String logEntry = "[" + timestamp + "] " + message;
    
    // Всегда выводим в Serial для отладки
    Serial.println(logEntry);
    
    // Пишем в файл если не debug или debug включен
    if (type != DEBUG_LOG || config.debugEnabled) {
        writeToFile(logEntry + "\n", getFilename(type));
    }
}

void DebugLogger::logSensor(float lux, bool relayState) {
    String timestamp = String(millis());
    String logEntry = "[" + timestamp + "] LUX:" + String(lux, 2) + 
                     " RELAY:" + (relayState ? "ON" : "OFF");
    
    writeToFile(logEntry + "\n", getFilename(SENSOR_LOG));
    
    // Дублируем в debug если включено
    if (config.debugEnabled) {
        DEBUG_LOG("📊 " + logEntry);
    }
}

void DebugLogger::writeToFile(const String& message, const String& filename) {
    // Определяем тип лога по имени файла
    LogType type = DEBUG_LOG;
    if (filename == "sensor.log") type = SENSOR_LOG;
    else if (filename == "events.log") type = EVENT_LOG;
    else if (filename == "system.log") type = SYSTEM_LOG;
    
    // Проверяем и ротируем если нужно
    rotateLogIfNeeded(type);
    
    String fullPath = "/logs/" + filename;
    
    File file = LittleFS.open(fullPath, "a");
    if (!file) {
        Serial.println("❌ Ошибка открытия файла: " + fullPath);
        return;
    }
    
    file.print(message);
    file.close();
    
    // Выводим в Serial для отладки
    Serial.print("LOG: " + filename + " - " + message);
}

String DebugLogger::getFilename(LogType type) {
    switch(type) {
        case DEBUG_LOG: return "debug.log";
        case SENSOR_LOG: return "sensor.log"; 
        case EVENT_LOG: return "events.log";
        case SYSTEM_LOG: return "system.log";
        default: return "unknown.log";
    }
}

void DebugLogger::enableDebug(bool enable) {
    config.debugEnabled = enable;
    saveConfig();
    EVENT_LOG("🔧 Отладка " + String(enable ? "включена" : "выключена"));
}

String DebugLogger::getLog(LogType type, uint16_t maxLines) {
    String fullPath = "/logs/" + getFilename(type);
    String result = "";
    uint16_t lineCount = 0;
    
    File file = LittleFS.open(fullPath, "r");
    if (!file) {
        return "Файл не найден: " + fullPath;
    }
    
    // Читаем файл и берем последние maxLines строк
    while (file.available() && lineCount < maxLines) {
        result = file.readStringUntil('\n') + "\n" + result;
        lineCount++;
    }
    
    file.close();
    return result;
}

void DebugLogger::clearLog(LogType type) {
    String fullPath = "/logs/" + getFilename(type);
    LittleFS.remove(fullPath);
    EVENT_LOG("🧹 Очищен лог: " + getFilename(type));
}

void DebugLogger::setMaxLogSize(uint32_t maxSize) {
    maxLogSize = maxSize;
    SYSTEM_LOG("🔧 Макс. размер лога установлен: " + String(maxSize) + " байт");
}

uint32_t DebugLogger::getLogSize(LogType type) {
    String fullPath = "/logs/" + getFilename(type);
    
    if (!LittleFS.exists(fullPath)) {
        return 0;
    }
    
    File file = LittleFS.open(fullPath, "r");
    if (!file) {
        return 0;
    }
    
    uint32_t size = file.size();
    file.close();
    return size;
}

void DebugLogger::rotateLogIfNeeded(LogType type) {
    String filename = getFilename(type);
    String fullPath = "/logs/" + filename;
    
    if (!LittleFS.exists(fullPath)) {
        return;
    }
    
    File file = LittleFS.open(fullPath, "r");
    if (!file) {
        return;
    }
    
    uint32_t currentSize = file.size();
    file.close();
    
    // Если файл меньше максимального размера - ничего не делаем
    if (currentSize <= maxLogSize) {
        return;
    }
    
    // Логируем в Serial (не в файл, чтобы избежать рекурсии)
    Serial.println("🔄 Ротация лога: " + filename + " (" + String(currentSize) + " байт)");
    
    // Читаем весь файл
    String allContent = "";
    File sourceFile = LittleFS.open(fullPath, "r");
    if (!sourceFile) {
        Serial.println("❌ Ошибка чтения файла для ротации");
        return;
    }
    
    allContent = sourceFile.readString();
    sourceFile.close();
    
    // Считаем общее количество строк
    int totalLines = 0;
    for (size_t i = 0; i < allContent.length(); i++) {
        if (allContent[i] == '\n') {
            totalLines++;
        }
    }
    
    // Если строк меньше 100, ничего не делаем
    if (totalLines <= 100) {
        Serial.println("📊 Строк меньше 100, ротация не нужна");
        return;
    }
    
    // Берем последние 80 строк (оставляем запас)
    int linesToKeep = 80;
    int linesFound = 0;
    String keptContent = "";
    
    // Идем с конца файла
    for (int i = allContent.length() - 1; i >= 0; i--) {
        if (allContent[i] == '\n') {
            linesFound++;
            if (linesFound > linesToKeep) {
                // Нашли достаточно строк, останавливаемся
                keptContent = allContent.substring(i + 1);
                break;
            }
        }
        
        // Если дошли до начала файла
        if (i == 0) {
            keptContent = allContent;
        }
    }
    
    // Перезаписываем файл
    File newFile = LittleFS.open(fullPath, "w");
    if (newFile) {
        newFile.print(keptContent);
        newFile.close();
        Serial.println("✅ Лог усечен: " + String(keptContent.length()) + " байт, " + String(linesToKeep) + " строк сохранено");
    } else {
        Serial.println("❌ Ошибка записи при ротации");
    }
}

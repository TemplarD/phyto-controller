// DebugLogger.h
#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <Arduino.h>

enum LogType {
    DEBUG_LOG,
    SENSOR_LOG, 
    EVENT_LOG,
    SYSTEM_LOG
};

class DebugLogger {
public:
    static void begin();
    static void log(const String& message, LogType type = DEBUG_LOG);
    static void logSensor(float lux, bool relayState);
    static void enableDebug(bool enable);
    static String getLog(LogType type, uint16_t maxLines = 50);
    static void clearLog(LogType type);

private:
    static void writeToFile(const String& message, const String& filename);
    static String getFilename(LogType type);
};

// Макросы для логирования
#define DEBUG_LOG(message) DebugLogger::log(message, DEBUG_LOG)
#define EVENT_LOG(message) DebugLogger::log(message, EVENT_LOG)
#define SYSTEM_LOG(message) DebugLogger::log(message, SYSTEM_LOG)

#endif
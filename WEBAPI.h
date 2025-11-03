// WebAPI.h
#ifndef WEB_API_H
#define WEB_API_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

class WebAPI {
public:
    void begin();
    void handleClient();
    
    // Статус системы
    String getSystemStatusJSON();
    
    // Управление
    void setManualMode(bool manualOn);
    void setAutoMode(bool autoMode);
    void setLightThreshold(float threshold);
    void toggleRelay();
    
    // Логи
    String getLogsJSON();
    
private:
    WebServer server;
    
    void setupRoutes();
    void handleRoot();
    void handleStatus();
    void handleControl();
    void handleSettings();
    void handleLogs();
    void handleNotFound();
    
    String getContentType(String filename);
    bool handleFileRead(String path);
};

extern WebAPI webAPI;

#endif
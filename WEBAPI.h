// WebAPI.h
#ifndef WEB_API_H
#define WEB_API_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

class WebAPI {
public:
    void begin();
    void handleClient();
    
private:
    WebServer server;
    
    void setupRoutes();
    void handleRoot();
    void handleStatus();
    void handleControl();
    void handleSettings();
    void handleLogs();
    void handleNotFound();
    
    String escapeJSONString(const String& input);
};

extern WebAPI webAPI;

#endif

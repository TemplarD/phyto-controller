// WebAPI.cpp
#include "WebAPI.h"
#include "Config.h"
#include "DebugLogger.h"
#include "LightSensor.h"
#include "RelayController.h"
#include <LittleFS.h>

// Добавляем extern объявления
extern LightSensor lightSensor;
extern RelayController relayController;
extern Settings config;

WebAPI webAPI;

void WebAPI::begin() {
    // Пока используем точку доступа без WiFi настроек
    Serial.println("Web API: Creating access point...");
    WiFi.softAP("PhytoController", "12345678");
    Serial.println("AP created: " + WiFi.softAPIP().toString());
    
    setupRoutes();
    server.begin();
    SYSTEM_LOG("Web server started on port 80");
}

void WebAPI::setupRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/api/control", HTTP_POST, [this]() { handleControl(); });
    server.on("/api/settings", HTTP_POST, [this]() { handleSettings(); });
    server.on("/api/logs", HTTP_GET, [this]() { handleLogs(); });
    
    server.onNotFound([this]() { handleNotFound(); });
}

void WebAPI::handleRoot() {
    String html = "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<title>PhytoController</title>";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<style>";
    html += "body { font-family: Arial; margin: 20px; background: #f0f0f0; }";
    html += ".card { background: white; padding: 20px; margin: 10px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
    html += ".status { font-size: 1.2em; margin: 10px 0; }";
    html += ".btn { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; }";
    html += ".btn-on { background: #4CAF50; color: white; }";
    html += ".btn-off { background: #f44336; color: white; }";
    html += ".btn-auto { background: #2196F3; color: white; }";
    html += ".log { background: #000; color: #0f0; padding: 10px; border-radius: 5px; font-family: monospace; height: 200px; overflow-y: scroll; }";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>PhytoController</h1>";
    
    html += "<div class=\"card\">";
    html += "<h2>System Status</h2>";
    html += "<div id=\"status\">Loading...</div>";
    html += "</div>";
    
    html += "<div class=\"card\">";
    html += "<h2>Control</h2>";
    html += "<button class=\"btn btn-on\" onclick=\"control('relay', true)\">ON Relay</button>";
    html += "<button class=\"btn btn-off\" onclick=\"control('relay', false)\">OFF Relay</button>";
    html += "<button class=\"btn btn-auto\" onclick=\"control('mode', true)\">AUTO Mode</button>";
    html += "<button class=\"btn\" onclick=\"control('mode', false)\">MANUAL Mode</button>";
    html += "</div>";
    
    html += "<div class=\"card\">";
    html += "<h2>Settings</h2>";
    html += "<label>Light threshold (lux):</label>";
    html += "<input type=\"number\" id=\"threshold\" value=\"500\">";
    html += "<button onclick=\"updateSettings()\">Save</button>";
    html += "</div>";
    
    html += "<div class=\"card\">";
    html += "<h2>Logs</h2>";
    html += "<div class=\"log\" id=\"logs\">Loading logs...</div>";
    html += "<button onclick=\"refreshLogs()\">Refresh Logs</button>";
    html += "</div>";
    
    html += "<script>";
    html += "function updateStatus() {";
    html += "  fetch('/api/status')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      document.getElementById('status').innerHTML = ";
    html += "        '<div class=\"status\">Relay: <b>' + (data.relayState ? 'ON' : 'OFF') + '</b></div>' +";
    html += "        '<div class=\"status\">Light: <b>' + data.lux.toFixed(2) + ' lux</b></div>' +";
    html += "        '<div class=\"status\">Mode: <b>' + (data.autoMode ? 'AUTO' : 'MANUAL') + '</b></div>' +";
    html += "        '<div class=\"status\">Threshold: <b>' + data.threshold + ' lux</b></div>' +";
    html += "        '<div class=\"status\">Uptime: <b>' + data.uptime + ' sec</b></div>';";
    html += "      document.getElementById('threshold').value = data.threshold;";
    html += "    });";
    html += "}";
    
    html += "function control(type, value) {";
    html += "  let body = {};";
    html += "  if (type === 'relay') { body = { relay: value }; }";
    html += "  else if (type === 'mode') { body = { autoMode: value }; }";
    html += "  fetch('/api/control', {";
    html += "    method: 'POST',";
    html += "    headers: {'Content-Type': 'application/json'},";
    html += "    body: JSON.stringify(body)";
    html += "  }).then(() => updateStatus());";
    html += "}";
    
    html += "function updateSettings() {";
    html += "  const threshold = document.getElementById('threshold').value;";
    html += "  fetch('/api/settings', {";
    html += "    method: 'POST',";
    html += "    headers: {'Content-Type': 'application/json'},";
    html += "    body: JSON.stringify({threshold: parseFloat(threshold)})";
    html += "  }).then(() => updateStatus());";
    html += "}";
    
    html += "function refreshLogs() {";
    html += "  fetch('/api/logs')";
    html += "    .then(response => response.json())";
    html += "    .then(data => {";
    html += "      document.getElementById('logs').innerText = data.logs;";
    html += "    });";
    html += "}";
    
    html += "setInterval(updateStatus, 3000);";
    html += "updateStatus();";
    html += "refreshLogs();";
    html += "</script>";
    html += "</body>";
    html += "</html>";
    
    server.send(200, "text/html", html);
}

void WebAPI::handleStatus() {
    // Ручное создание JSON
    String json = "{";
    json += "\"relayState\":" + String(relayController.getState() ? "true" : "false") + ",";
    json += "\"lux\":" + String(lightSensor.getLux()) + ",";
    json += "\"autoMode\":" + String(config.autoMode ? "true" : "false") + ",";
    json += "\"threshold\":" + String(config.lightThreshold) + ",";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"sensorAvailable\":" + String(lightSensor.isAvailable() ? "true" : "false") + ",";
    json += "\"wifiStatus\":\"" + String(WiFi.status() == WL_CONNECTED ? "connected" : "ap") + "\"";
    json += "}";
    
    server.send(200, "application/json", json);
}

void WebAPI::handleControl() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        
        // Простой парсинг JSON вручную
        if (body.indexOf("\"relay\":true") != -1) {
            relayController.turnOn();
        } else if (body.indexOf("\"relay\":false") != -1) {
            relayController.turnOff();
        }
        
        if (body.indexOf("\"autoMode\":true") != -1) {
            config.autoMode = true;
            saveConfig();
        } else if (body.indexOf("\"autoMode\":false") != -1) {
            config.autoMode = false;
            saveConfig();
        }
        
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
}

void WebAPI::handleSettings() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        
        // Простой парсинг threshold
        int thresholdIndex = body.indexOf("\"threshold\":");
        if (thresholdIndex != -1) {
            int start = thresholdIndex + 12; // после "threshold":
            int end = body.indexOf(",", start);
            if (end == -1) end = body.indexOf("}", start);
            
            if (end != -1) {
                String thresholdStr = body.substring(start, end);
                config.lightThreshold = thresholdStr.toFloat();
                saveConfig();
                EVENT_LOG("Light threshold set: " + String(config.lightThreshold) + " lux");
            }
        }
        
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
}

void WebAPI::handleLogs() {
    String logs = DebugLogger::getLog(DEBUG_LOG, 50);
    String json = "{\"logs\":\"" + escapeJSONString(logs) + "\"}";
    server.send(200, "application/json", json);
}

String WebAPI::escapeJSONString(const String& input) {
    String result = input;
    result.replace("\\", "\\\\");
    result.replace("\"", "\\\"");
    result.replace("\n", "\\n");
    result.replace("\r", "\\r");
    result.replace("\t", "\\t");
    return result;
}

void WebAPI::handleNotFound() {
    server.send(404, "text/plain", "File Not Found");
}

void WebAPI::handleClient() {
    server.handleClient();
}

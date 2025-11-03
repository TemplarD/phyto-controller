// WebAPI.cpp
#include "WebAPI.h"
#include "Config.h"
#include "DebugLogger.h"
#include "LightSensor.h"
#include "RelayController.h"

WebAPI webAPI;

void WebAPI::begin() {
    // Настройка WiFi (позже добавим WiFiManager)
    WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());
    
    Serial.print("📡 Подключение к WiFi");
    for(int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
        delay(500);
        Serial.print(".");
    }
    
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi подключен: " + WiFi.localIP().toString());
        SYSTEM_LOG("📡 WiFi подключен: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n❌ Ошибка WiFi");
        // Создаем точку доступа
        WiFi.softAP("PhytoController", "12345678");
        Serial.println("📶 AP создан: 192.168.4.1");
    }
    
    setupRoutes();
    server.begin();
    SYSTEM_LOG("🌐 Web сервер запущен");
}

void WebAPI::setupRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/api/control", HTTP_POST, [this]() { handleControl(); });
    server.on("/api/settings", HTTP_POST, [this]() { handleSettings(); });
    server.on("/api/logs", HTTP_GET, [this]() { handleLogs(); });
    
    // Статические файлы для веб-интерфейса
    server.onNotFound([this]() { handleNotFound(); });
}

void WebAPI::handleRoot() {
    String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
        <title>🌱 PhytoController</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body { font-family: Arial; margin: 20px; background: #f0f0f0; }
            .card { background: white; padding: 20px; margin: 10px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
            .status { font-size: 1.2em; margin: 10px 0; }
            .btn { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; }
            .btn-on { background: #4CAF50; color: white; }
            .btn-off { background: #f44336; color: white; }
            .btn-auto { background: #2196F3; color: white; }
            .log { background: #000; color: #0f0; padding: 10px; border-radius: 5px; font-family: monospace; height: 200px; overflow-y: scroll; }
        </style>
    </head>
    <body>
        <h1>🌱 PhytoController</h1>
        
        <div class="card">
            <h2>📊 Статус системы</h2>
            <div id="status">Загрузка...</div>
        </div>
        
        <div class="card">
            <h2>🎛️ Управление</h2>
            <button class="btn btn-on" onclick="setRelay(true)">ВКЛ Реле</button>
            <button class="btn btn-off" onclick="setRelay(false)">ВЫКЛ Реле</button>
            <button class="btn btn-auto" onclick="setMode(true)">АВТО режим</button>
            <button class="btn" onclick="setMode(false)">РУЧНОЙ режим</button>
        </div>
        
        <div class="card">
            <h2>⚙️ Настройки</h2>
            <label>Порог освещенности (lux):</label>
            <input type="number" id="threshold" value="500">
            <button onclick="updateSettings()">Сохранить</button>
        </div>
        
        <div class="card">
            <h2>📋 Логи</h2>
            <div class="log" id="logs">Загрузка логов...</div>
            <button onclick="refreshLogs()">Обновить логи</button>
        </div>
        
        <script>
            async function updateStatus() {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                document.getElementById('status').innerHTML = `
                    <div class="status">💡 Реле: <b>${data.relayState ? 'ВКЛ' : 'ВЫКЛ'}</b></div>
                    <div class="status">🔆 Освещенность: <b>${data.lux.toFixed(2)} lux</b></div>
                    <div class="status">🤖 Режим: <b>${data.autoMode ? 'АВТО' : 'РУЧНОЙ'}</b></div>
                    <div class="status">🎯 Порог: <b>${data.threshold} lux</b></div>
                    <div class="status">⏰ Uptime: <b>${data.uptime} сек</b></div>
                `;
                
                document.getElementById('threshold').value = data.threshold;
            }
            
            async function setRelay(state) {
                await fetch('/api/control', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({relay: state})
                });
                updateStatus();
            }
            
            async function setMode(auto) {
                await fetch('/api/control', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({autoMode: auto})
                });
                updateStatus();
            }
            
            async function updateSettings() {
                const threshold = document.getElementById('threshold').value;
                await fetch('/api/settings', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({threshold: parseFloat(threshold)})
                });
                updateStatus();
            }
            
            async function refreshLogs() {
                const response = await fetch('/api/logs');
                const data = await response.json();
                document.getElementById('logs').innerText = data.logs;
            }
            
            // Обновляем статус каждые 3 секунды
            setInterval(updateStatus, 3000);
            updateStatus();
            refreshLogs();
        </script>
    </body>
    </html>
    )";
    
    server.send(200, "text/html", html);
}

void WebAPI::handleStatus() {
    DynamicJsonDocument doc(1024);
    
    doc["relayState"] = relayController.getState();
    doc["lux"] = lightSensor.getLux();
    doc["autoMode"] = config.autoMode;
    doc["threshold"] = config.lightThreshold;
    doc["uptime"] = millis() / 1000;
    doc["sensorAvailable"] = lightSensor.isAvailable();
    doc["wifiStatus"] = WiFi.status() == WL_CONNECTED ? "connected" : "disconnected";
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebAPI::handleControl() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        DynamicJsonDocument doc(256);
        deserializeJson(doc, body);
        
        if (doc.containsKey("relay")) {
            bool relayState = doc["relay"];
            if (relayState) relayController.turnOn();
            else relayController.turnOff();
        }
        
        if (doc.containsKey("autoMode")) {
            config.autoMode = doc["autoMode"];
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
        DynamicJsonDocument doc(256);
        deserializeJson(doc, body);
        
        if (doc.containsKey("threshold")) {
            config.lightThreshold = doc["threshold"];
            saveConfig();
            EVENT_LOG("🎯 Установлен порог освещенности: " + String(config.lightThreshold) + " lux");
        }
        
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
}

void WebAPI::handleLogs() {
    DynamicJsonDocument doc(2048);
    doc["logs"] = DebugLogger::getLog(DEBUG_LOG, 50);
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebAPI::handleNotFound() {
    if (!handleFileRead(server.uri())) {
        server.send(404, "text/plain", "File Not Found");
    }
}

bool WebAPI::handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    
    if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
        if (LittleFS.exists(pathWithGz)) {
            path = pathWithGz;
        }
        
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

String WebAPI::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    if (filename.endsWith(".css")) return "text/css";
    if (filename.endsWith(".js")) return "application/javascript";
    return "text/plain";
}

void WebAPI::handleClient() {
    server.handleClient();
}
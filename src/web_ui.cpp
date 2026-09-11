#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "web_ui.h"
#include "web_theme.h"
#include "web_main.h"
#include "server_comm.h"
#include <ArduinoJson.h>
#include <time.h>
#include <Preferences.h>

extern AsyncWebServer server;

// Külső függvények deklarációi
extern void handleConfigPage(AsyncWebServerRequest *request);
extern void handleConnections(AsyncWebServerRequest *request);
extern void handleSensors(AsyncWebServerRequest *request); 
extern void handleDiag(AsyncWebServerRequest *request);    
extern void handleApiApInfo(AsyncWebServerRequest *request);
extern void handleApiSaveAp(AsyncWebServerRequest *request);
extern void handleApiSaveSta(AsyncWebServerRequest *request);
extern void handleApiSaveRadio(AsyncWebServerRequest *request);
extern void handleApiScanWifi(AsyncWebServerRequest *request);
extern void handleApiTelemetry(AsyncWebServerRequest *request);

// --- ÚJ: Szerver konfiguráció és gyors csatlakozás végpontok deklarációi ---
extern void handleApiSaveServer(AsyncWebServerRequest *request);
extern void handleApiQuickConnect(AsyncWebServerRequest *request);

void initWebUI() {
    server.on("/s.css", HTTP_GET, handleCss);
    
    // Főoldal és fülek
    server.on("/", HTTP_GET, handleRoot);
    server.on("/connections", HTTP_GET, handleConnections);
    server.on("/sensors", HTTP_GET, handleSensors); 
    server.on("/config", HTTP_GET, handleConfigPage); 
    
    // API Végpontok
    server.on("/api/telemetry", HTTP_GET, handleApiTelemetry);
    server.on("/api/ap_info", HTTP_GET, handleApiApInfo);
    server.on("/api/save_ap", HTTP_POST, handleApiSaveAp);
    server.on("/api/save_sta", HTTP_POST, handleApiSaveSta);
    server.on("/api/save_radio", HTTP_POST, handleApiSaveRadio);
    server.on("/api/scan_wifi", HTTP_GET, handleApiScanWifi);
    
    // --- ÚJ: Végpontok beregisztrálása ---
    server.on("/api/save_server", HTTP_POST, handleApiSaveServer);
    server.on("/api/quick_connect", HTTP_POST, handleApiQuickConnect);

    server.onNotFound([](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/"); // Visszairányítás az AP alapértelmezett IP-jére
    });
}

void handleApiSaveServer(AsyncWebServerRequest *request) {
    if (request->hasParam("mac", true) && request->hasParam("chan", true)) {
        String macStr = request->getParam("mac", true)->value();
        int chan = request->getParam("chan", true)->value().toInt();
        
        // MAC cím parszeolása stringből (pl. AA:BB:CC:DD:EE:FF)
        uint8_t macBytes[6];
        int values[6];
        if (6 == sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5])) {
            for(int i = 0; i < 6; ++i) {
                macBytes[i] = (uint8_t)values[i];
            }
            
            Preferences serverPrefs;
            serverPrefs.begin("kaptar", false);
            serverPrefs.putBytes("srvMac", macBytes, 6);
            serverPrefs.putUInt("channel", chan);
            serverPrefs.end();
            
            request->send(200, "text/plain", "OK");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "Hibás MAC formátum");
        }
    } else {
        request->send(400, "text/plain", "Hiányzó paraméterek");
    }
}

void handleApiQuickConnect(AsyncWebServerRequest *request) {
    Preferences serverPrefs;
    serverPrefs.begin("kaptar", false);
    
    // Töröljük a mentett MAC címet. Így induláskor a main.cpp nem fogja 
    // megtalálni, és automatikusan lefuttatja a scanAndSyncServer() függvényt!
    serverPrefs.remove("srvMac");
    serverPrefs.end();
    
    request->send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
}


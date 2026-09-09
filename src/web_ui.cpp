#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "web_ui.h"
#include "web_theme.h"

extern AsyncWebServer server;

// --- Extern deklarációk a későbbi modulokhoz ---
extern void handleRoot(AsyncWebServerRequest *request);
extern void handleConfigPage(AsyncWebServerRequest *request);

// API végpontok (később a web_config modulba kerülnek)
extern void handleApiApInfo(AsyncWebServerRequest *request);
extern void handleApiSaveAp(AsyncWebServerRequest *request);
extern void handleApiSaveSta(AsyncWebServerRequest *request);
extern void handleApiSaveRadio(AsyncWebServerRequest *request);
extern void handleApiScanWifi(AsyncWebServerRequest *request);

void initWebUI() {
    // Alap UI fájlok
    server.on("/s.css", HTTP_GET, handleCss);
    
    // Weboldalak
    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_GET, handleConfigPage);
    
    // API Végpontok
    server.on("/api/ap_info", HTTP_GET, handleApiApInfo);
    server.on("/api/save_ap", HTTP_POST, handleApiSaveAp);
    server.on("/api/save_sta", HTTP_POST, handleApiSaveSta);
    server.on("/api/save_radio", HTTP_POST, handleApiSaveRadio);
    server.on("/api/scan_wifi", HTTP_GET, handleApiScanWifi);

    // Captive Portal middleware
    server.onNotFound([](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });
}
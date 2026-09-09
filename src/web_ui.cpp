#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "web_ui.h"
#include "web_theme.h"
#include "web_main.h"

extern AsyncWebServer server;

// Külső függvények deklarációi
extern void handleConfigPage(AsyncWebServerRequest *request);
extern void handleConnections(AsyncWebServerRequest *request);
extern void handleSensors(AsyncWebServerRequest *request); // <--- Szenzorok deklarációja
extern void handleDiag(AsyncWebServerRequest *request);    // Ha külön modulban van
extern void handleApiApInfo(AsyncWebServerRequest *request);
extern void handleApiSaveAp(AsyncWebServerRequest *request);
extern void handleApiSaveSta(AsyncWebServerRequest *request);
extern void handleApiSaveRadio(AsyncWebServerRequest *request);
extern void handleApiScanWifi(AsyncWebServerRequest *request);
extern void handleApiTelemetry(AsyncWebServerRequest *request);

void initWebUI() {
    server.on("/s.css", HTTP_GET, handleCss);
    
    // Főoldal és fülek
    server.on("/", HTTP_GET, handleRoot);
    server.on("/connections", HTTP_GET, handleConnections);
    server.on("/sensors", HTTP_GET, handleSensors); // <--- Itt regisztráljuk a szenzorokat
    
    // API Végpontok
    server.on("/api/telemetry", HTTP_GET, handleApiTelemetry);
    server.on("/api/ap_info", HTTP_GET, handleApiApInfo);
    server.on("/api/save_ap", HTTP_POST, handleApiSaveAp);
    server.on("/api/save_sta", HTTP_POST, handleApiSaveSta);
    server.on("/api/save_radio", HTTP_POST, handleApiSaveRadio);
    server.on("/api/scan_wifi", HTTP_GET, handleApiScanWifi);

    server.onNotFound([](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });
}
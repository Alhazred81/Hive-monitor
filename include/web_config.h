#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void handleConnections(AsyncWebServerRequest *request);
void handleApiApInfo(AsyncWebServerRequest *request);
void handleApiSaveAp(AsyncWebServerRequest *request);
void handleApiSaveSta(AsyncWebServerRequest *request);
void handleApiSaveRadio(AsyncWebServerRequest *request);
void handleApiScanWifi(AsyncWebServerRequest *request);
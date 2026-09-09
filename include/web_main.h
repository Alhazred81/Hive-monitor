#pragma once
#include <ESPAsyncWebServer.h>

void handleRoot(AsyncWebServerRequest *request);
void handleApiTelemetry(AsyncWebServerRequest *request);
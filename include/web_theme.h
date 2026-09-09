#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

String htmlHead(const String& title, const String& activeTab);
String htmlFoot();
void handleCss(AsyncWebServerRequest *request);
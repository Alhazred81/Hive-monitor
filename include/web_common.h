#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

extern AsyncWebServer server;
extern DNSServer dnsServer;
extern Preferences prefs;
extern String fullMac;

void initWeb();
void loopWeb();
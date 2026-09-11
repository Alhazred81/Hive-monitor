#pragma once
#include <Arduino.h>

// Globális változók a kapcsolat és az idő állapotához
extern bool serverFound;
extern volatile unsigned long lastSyncTimeMillis;
extern volatile bool timeSynchronized;

void initServerComm();
void scanAndSyncServer();
void sendTelemetryJson(float temp, float hum, float pres, float zcr, uint8_t state, double bands[8]);
extern String currentRadioMode;

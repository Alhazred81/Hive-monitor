//server_comm.h

#pragma once
#include <Arduino.h>

extern bool serverFound;
extern uint8_t serverMac[6];

void initServerComm();
void scanAndSyncServer();
bool sendTelemetryJson(float temp, float hum, float pres, float zcr, uint8_t state, const double* bands);
//sensors.h

#pragma once
#include <Arduino.h>

extern float currentTemp;
extern float currentHum;
extern float currentPres;
extern double currentBands[8];
extern volatile bool motionDetected;
bool checkMpuSeverity();

void initSensors();
void updateSensors();

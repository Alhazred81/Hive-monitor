#pragma once
#include <Arduino.h>

struct SensorState {
    bool enabled = true;
    bool lastReadOk = false;
    String value = "--";
};

extern SensorState gAht20;
extern SensorState gBmp280;
extern SensorState gSgp41;
extern SensorState gInmp441_1;
extern SensorState gInmp441_2;
extern SensorState gSht40;

extern float currentTemp;
extern float currentHum;
extern float currentPres;
extern double currentBands[8];
extern volatile bool motionDetected;

void initSensors();
void updateSensors();
void recoverI2CBus();
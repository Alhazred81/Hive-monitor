#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include "sensors.h"

float currentTemp = 0.0f;
float currentHum = 0.0f;
float currentPres = 0.0f;
double currentBands[8] = {0};
volatile bool motionDetected = false;
volatile bool isScanning = false;

SensorState gAht20;
SensorState gBmp280;
SensorState gSgp41;
SensorState gInmp441_1;
SensorState gInmp441_2;
SensorState gSht40;

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

void recoverI2CBus() {
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    delay(20);

    if (digitalRead(SDA) == LOW) {
        pinMode(SCL, OUTPUT);
        for (int i = 0; i < 9; i++) {
            digitalWrite(SCL, LOW);
            delayMicroseconds(5);
            digitalWrite(SCL, HIGH);
            delayMicroseconds(5);
        }
    }
    pinMode(SCL, INPUT_PULLUP);
}

void initSensors() {
    recoverI2CBus();
    Wire.begin();

    if (aht.begin()) {
        gAht20.enabled = true;
    } else {
        gAht20.enabled = false;
    }

    // BMP280 alapértelmezett címe 0x76 vagy 0x77
    if (bmp.begin(0x76) || bmp.begin(0x77)) {
        gBmp280.enabled = true;
    } else {
        gBmp280.enabled = false;
    }

    gSgp41.enabled = false;   // Jelenleg nincs rákötve
    gInmp441_1.enabled = true; // 1 db mikrofon aktív
    gInmp441_2.enabled = false;
    gSht40.enabled = false;   // Jelenleg nincs rákötve
}

void updateSensors() {
    if (isScanning) return;

    // AHT20 olvasás
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
        currentTemp = temp.temperature;
        currentHum = humidity.relative_humidity;
        gAht20.lastReadOk = true;
        gAht20.value = String(currentTemp, 1) + " °C / " + String(currentHum, 1) + " %";
    } else {
        gAht20.lastReadOk = false;
        gAht20.value = "Hiba / Nincs jel";
    }

    // BMP280 olvasás
    float press = bmp.readPressure();
    if (!isnan(press)) {
        currentPres = press / 100.0F; // Pa -> hPa
        gBmp280.lastReadOk = true;
        gBmp280.value = String(currentPres, 1) + " hPa";
    } else {
        gBmp280.lastReadOk = false;
        gBmp280.value = "Hiba / Nincs jel";
    }

    // Alapértelmezett státuszok a nem csatlakoztatott eszközökhöz
    gSgp41.lastReadOk = false;
    gSgp41.value = "Nincs bekötve";

    gSht40.lastReadOk = false;
    gSht40.value = "Nincs bekötve";

    gInmp441_1.lastReadOk = true;
    gInmp441_1.value = "Aktív (I2S)";

    gInmp441_2.lastReadOk = false;
    gInmp441_2.value = "Nincs konfigurálva";
}


bool checkMpuSeverity() {
    return true;
}
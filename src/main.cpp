#include <Arduino.h>
#include <Wire.h>

#include "analyzer.h"
#include "config.h"
#include "connections.h"
#include "sensors.h"
#include "server_comm.h"
#include "spec_ana.h"
#include "storage.h"
#include "web_common.h"
#include "web_diag.h"

#define WAKE_PIN 3 

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Az I²C buszt minden induláskor fixen inicializáljuk
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    initSensors();
    recoverI2CBus();
    initAnalyzer();
    initSpecAna();
    initStorage();
    initDiagRoutes();
    handleConnections();

    // 1. Kommunikáció inicializálása és ESP-NOW szinkronizálás
    initServerComm();

    if (!serverFound) {
        Serial.println("Szerver keresése ESP-NOW csatornákon...");
        scanAndSyncServer();
    } else {
        Serial.println("Szerver megtalálva a mentett adatok alapján.");
    }

    // 2. A webszerver (és az AP/STA mód) elindítása CSAK a szkennelés UTÁN!
    initWeb();

    #ifndef CONFIG_IDF_TARGET_ESP32C3
        esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
    #endif
}

void loop() {
    updateSensors();
    updateSpecAna();
    updateAnalyzer();
    loopWeb();
    handleClientTick();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 10000) {
        lastUpdate = millis();

        HiveRecord record;
        record.timestamp = millis();
        record.temp = currentTemp;
        record.hum = currentHum;
        record.pres = currentPres;
        record.zcr = currentZCR;
        record.state = (uint8_t)currentHiveState;
        for (int i = 0; i < 8; i++) {
            record.bands[i] = currentBands[i];
        }
        appendHiveRecord(record);
        cleanupOldLogs(14);

        if (serverFound) {
            sendTelemetryJson(currentTemp, currentHum, currentPres, currentZCR, (uint8_t)currentHiveState, currentBands);
        }

        Serial.printf("\rH: %.1fC | P: %.0f%% | Ny: %.0fhPa | ZCR: %.0f | dB: %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f          ", 
            currentTemp, currentHum, currentPres, currentZCR,
            currentBands[0], currentBands[1], currentBands[2], currentBands[3], 
            currentBands[4], currentBands[5], currentBands[6], currentBands[7]);
    }

     // Létfontosságú, hogy a ciklus végén engedjünk időt a háttérszálaknak (AsyncTCP)
     yield();
}
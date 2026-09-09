#include <Arduino.h>
#include <Wire.h>

#include "analyzer.h"
#include "config.h"
#include "sensors.h"
#include "server_comm.h"
#include "spec_ana.h"
#include "storage.h"
#include "web_common.h"

#define WAKE_PIN 3 

void setup() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0 || wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 || wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        
        if (!checkMpuSeverity()) {
            #ifndef CONFIG_IDF_TARGET_ESP32C3
                esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
            #endif
            esp_deep_sleep_start();
        }
    }

    Serial.begin(115200);
    delay(1000);
    
    initSensors();
    initAnalyzer();
    initSpecAna();
    initStorage();

    // WEB ÉS AP INDÍTÁSA
    initWeb();

    initServerComm();

    if (!serverFound) {
        Serial.println("Szerver keresése ESP-NOW csatornákon...");
        scanAndSyncServer();
    } else {
        Serial.println("Szerver megtalálva a mentett adatok alapján.");
    }

    #ifndef CONFIG_IDF_TARGET_ESP32C3
        esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
    #endif
}

void loop() {
    // Ezeknek folyamatosan, blokkolás nélkül kell futniuk
    updateSensors();
    updateSpecAna();
    updateAnalyzer();
    loopWeb();

    // Nem blokkoló időzítő: csak 10 másodpercenként fut le ez a blokk
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
            bool success = sendTelemetryJson(currentTemp, currentHum, currentPres, currentZCR, (uint8_t)currentHiveState, currentBands);
            if (success) {
                Serial.println("Telemetria sikeresen elküldve a szervernek.");
            } else {
                Serial.println("Hiba az ESP-NOW adatküldés során.");
            }
        }

        // Egyetlen soros kiíratás kocsivisszával (\r) és szóközökkel a sor végén
        Serial.printf("\rH: %.1fC | P: %.0f%% | Ny: %.0fhPa | ZCR: %.0f | dB: %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f          ", 
            currentTemp, currentHum, currentPres, currentZCR,
            currentBands[0], currentBands[1], currentBands[2], currentBands[3], 
            currentBands[4], currentBands[5], currentBands[6], currentBands[7]);
    }
}
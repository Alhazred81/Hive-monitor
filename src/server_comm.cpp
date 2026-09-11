#include "server_comm.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <sys/time.h>

bool serverFound = false;
uint8_t serverMac[6] = {0};
uint8_t serverChannel = 1;

// Rádió mód tárolása
String currentRadioMode = "espnow"; 

// Időszinkronizáció változói
volatile unsigned long lastSyncTimeMillis = 0;
volatile bool timeSynchronized = false;

Preferences serverPrefs;
volatile bool gPairingSuccess = false;

#pragma pack(push, 1)
struct PairingData {
    uint32_t magic;         // 0x42454553 ("BEES")
    uint8_t  msgType;       // 0x01
    uint8_t  serverMac[6];
    char     ssid[32];
    char     password[32];
    uint32_t unixTime;
};
#pragma pack(pop)

// ESP-NOW vétel callback a kaptármonitoron
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    if (data_len == sizeof(PairingData)) {
        PairingData* packet = (PairingData*)data;
        
        // Ellenőrizzük a varázsszót és a típus azonosítót
        if (packet->magic == 0x42454553 && packet->msgType == 0x01) {
            uint8_t primaryChan;
            wifi_second_chan_t secondChan;
            esp_wifi_get_channel(&primaryChan, &secondChan);
            
            // Adatok mentése a flash memóriába
            serverPrefs.begin("kaptar", false);
            serverPrefs.putBytes("srvMac", packet->serverMac, 6);
            serverPrefs.putUInt("channel", primaryChan);
            serverPrefs.putString("ssid", packet->ssid);
            serverPrefs.putString("pass", packet->password);
            serverPrefs.end();
            
            memcpy(serverMac, packet->serverMac, 6);
            serverChannel = primaryChan;
            gPairingSuccess = true;

            // Belső óra beállítása a szerverről kapott idő alapján
            if (packet->unixTime > 1000000000) { // Alapvető validálás (2001 utáni idő)
                struct timeval tv;
                tv.tv_sec = packet->unixTime;
                tv.tv_usec = 0;
                settimeofday(&tv, NULL);
                lastSyncTimeMillis = millis();
                timeSynchronized = true;
            }
        }
    }
}

void initServerComm() {
    serverPrefs.begin("kaptar", true);
    
    // Rádió mód kiolvasása (alapértelmezett: "espnow")
    currentRadioMode = serverPrefs.getString("radio", "espnow");
    
    if (serverPrefs.getBytesLength("srvMac") == 6) {
        serverPrefs.getBytes("srvMac", serverMac, 6);
        serverChannel = serverPrefs.getUInt("channel", 1);
        serverFound = true;
    } else {
        serverFound = false;
    }
    serverPrefs.end();
}

void scanAndSyncServer() {
    gPairingSuccess = false;
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW inicializálási hiba!");
        return;
    }
    
    esp_now_register_recv_cb(onReceive);
    
    const int HOPPING_DELAY_MS = 250; // 250 ms csatornánként
    
    for (int ch = 1; ch <= 13; ch++) {
        if (gPairingSuccess) {
            break; 
        }
        
        esp_wifi_set_promiscuous(true);
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        esp_wifi_set_promiscuous(false);
        
        unsigned long startWait = millis();
        while (millis() - startWait < HOPPING_DELAY_MS) {
            if (gPairingSuccess) {
                break;
            }
            delay(10);
            yield(); // Létfontosságú a WDT miatt!
        }
    }
    
    esp_now_deinit();
    
    if (gPairingSuccess) {
        serverFound = true;
        Serial.println("\n[SYNC] Párosítás sikeres! Adatok és idő mentve.");
    } else {
        serverFound = false;
        Serial.println("\n[SYNC] Nem talált szervert a 13 csatornán.");
    }
}

void sendTelemetryJson(float temp, float hum, float pres, float zcr, uint8_t state, double bands[8]) {
    if (!serverFound) return;

    // JsonDocument az ArduinoJson 7-hez
    JsonDocument doc;
    doc["temp"] = temp;
    doc["hum"] = hum;
    doc["pres"] = pres;
    doc["zcr"] = zcr;
    doc["state"] = state;
    
    JsonArray bandsArray = doc["bands"].to<JsonArray>();
    for (int i = 0; i < 8; i++) {
        bandsArray.add(bands[i]);
    }

    String output;
    serializeJson(doc, output);
    
    // Itt történik a tényleges küldés (később implementálandó a választott protokoll szerint)
}
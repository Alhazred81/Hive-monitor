//web_comm.cpp

#include "server_comm.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <ArduinoJson.h>

Preferences commPrefs;
bool serverFound = false;
uint8_t serverMac[6] = {0};
uint8_t currentChannel = 1;
String deviceMacStr = "";

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if (len >= sizeof(uint32_t)) {
        uint32_t serverTime;
        memcpy(&serverTime, incomingData, sizeof(uint32_t));
        
        memcpy(serverMac, mac, 6);
        serverFound = true;
    }
}

void initServerComm() {
    WiFi.mode(WIFI_AP_STA);
    deviceMacStr = WiFi.macAddress();

    if (esp_now_init() != ESP_OK) {
        return;
    }
    esp_now_register_recv_cb(onDataRecv);

    commPrefs.begin("espnow_cfg", true);
    bool hasSaved = commPrefs.getBytes("server_mac", serverMac, 6) == 6;
    currentChannel = commPrefs.getUChar("channel", 1);
    commPrefs.end();

    if (hasSaved) {
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, serverMac, 6);
        peerInfo.channel = currentChannel;
        peerInfo.encrypt = false;
        esp_now_add_peer(&peerInfo);
        serverFound = true; 
    }
}

void scanAndSyncServer() {
    if (serverFound) return;

    esp_now_peer_info_t broadcastPeer = {};
    memset(broadcastPeer.peer_addr, 0xFF, 6);
    broadcastPeer.channel = 0;
    broadcastPeer.encrypt = false;
    esp_now_add_peer(&broadcastPeer);

    while (!serverFound && currentChannel <= 13) {
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
        
        uint8_t pingMsg[] = "jollerihijohoko";
        esp_now_send(broadcastPeer.peer_addr, pingMsg, sizeof(pingMsg));
        
        delay(150); 
        if (!serverFound) {
            currentChannel++;
        }
    }

    if (serverFound) {
        commPrefs.begin("espnow_cfg", false);
        commPrefs.putBytes("server_mac", serverMac, 6);
        commPrefs.putUChar("channel", currentChannel);
        commPrefs.end();
    }
    
    esp_now_del_peer(broadcastPeer.peer_addr);
}

bool sendTelemetryJson(float temp, float hum, float pres, float zcr, uint8_t state, const double* bands) {
    if (!serverFound) return false;

    // ArduinoJson v7 ajánlott JsonDocument szintaxisa
    JsonDocument doc;
    doc["id"] = deviceMacStr;
    doc["t"] = millis();
    doc["tp"] = temp;
    doc["hm"] = hum;
    doc["st"] = state;
    doc["zc"] = zcr;

    JsonArray bArray = doc["b"].to<JsonArray>();
    for(int i = 0; i < 8; i++) {
        bArray.add((int)bands[i]);
    }

    char buffer[250];
    size_t n = serializeJson(doc, buffer);

    esp_err_t result = esp_now_send(serverMac, (uint8_t*)buffer, n);
    return (result == ESP_OK);
}
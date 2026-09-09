#include "web_common.h"
#include "config.h"
#include <WiFi.h>
#include <DNSServer.h>
#include "web_main.h"
#include "web_sensors.h"
#include <Preferences.h>

AsyncWebServer server(80);
DNSServer dnsServer;
Preferences prefs;

String currentSSID;
String currentPass;
String fullMac;

void initWeb() {
    WiFi.mode(WIFI_AP);
    
    fullMac = WiFi.macAddress();
    String macClean = fullMac;
    macClean.replace(":", "");
    String defaultSSID = "Monitor_" + macClean.substring(macClean.length() - 4);

    // Itt kell 'false'-ra cserélni a 'true'-t
    prefs.begin("wifi_cfg", false); 
    currentSSID = prefs.getString("ap_ssid", defaultSSID);
    currentPass = prefs.getString("ap_pass", DEFAULT_AP_PASS);
    prefs.end();
    
    WiFi.softAP(currentSSID.c_str(), currentPass.c_str());
    dnsServer.start(DNS_PORT_NUM, "*", WiFi.softAPIP());

    initWebMain();
    initWebSensors();

    // Új végpont az AP adatok lekéréséhez
    server.on("/api/ap_info", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{\"mac\":\"" + fullMac + "\",\"ssid\":\"" + currentSSID + "\"}";
        request->send(200, "application/json", json);
    });

    // Új végpont a beállítások mentéséhez
    server.on("/api/save_ap", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
            String newSsid = request->getParam("ssid", true)->value();
            String newPass = request->getParam("pass", true)->value();
            
            prefs.begin("wifi_cfg", false); // false = írás/olvasás
            prefs.putString("ap_ssid", newSsid);
            prefs.putString("ap_pass", newPass);
            prefs.end();
            
            request->send(200, "text/plain", "OK");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "Hiba");
        }
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        request->redirect("http://192.168.4.1/");
    });

    server.begin();
}

void loopWeb() {
    dnsServer.processNextRequest();
}
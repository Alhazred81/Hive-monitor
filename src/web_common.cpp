#include "web_common.h"
#include <WiFi.h>
#include "config.h"
#include "web_ui.h"

AsyncWebServer server(80);
DNSServer dnsServer;
Preferences prefs;
String fullMac;

void initWeb() {
    WiFi.mode(WIFI_AP_STA);
    Serial.println("[WEB] WiFi mód beállítva: WIFI_AP_STA");
    
    prefs.begin("wifi_cfg", false); 
    String staSsid = prefs.getString("sta_ssid", "");
    String staPass = prefs.getString("sta_pass", "");

    fullMac = WiFi.macAddress();
    String macClean = fullMac;
    macClean.replace(":", "");
    String defaultSSID = "Monitor_" + macClean.substring(macClean.length() - 4);
    String currentSSID = prefs.getString("ap_ssid", defaultSSID);
    String currentPass = prefs.getString("ap_pass", DEFAULT_AP_PASS);
    prefs.end();

    if (staSsid.length() > 0) {
        Serial.printf("[WEB] Csatlakozás teszt hálózathoz: %s\n", staSsid.c_str());
        WiFi.begin(staSsid.c_str(), staPass.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("[WEB] Kliens IP cím: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("[WEB] Időtúllépés, nem sikerült felcsatlakozni a Wi-Fi-re.");
        }
    }

    if (WiFi.softAP(currentSSID.c_str(), currentPass.c_str())) {
        Serial.printf("[WEB] AP sikeresen elindult. SSID: %s\n", currentSSID.c_str());
    } else {
        Serial.println("[WEB] Hiba: Az AP indítása sikertelen.");
    }

    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
        if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
            Serial.println("[WEB-DIAG] Új kliens csatlakozott az AP-hoz.");
        } else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
            Serial.println("[WEB-DIAG] Kliens levált az AP-ról.");
        }
    });

    if (dnsServer.start(DNS_PORT_NUM, "*", WiFi.softAPIP())) {
        Serial.println("[WEB] DNS szerver (Captive Portal) elindítva.");
    }

    // A végpontok regisztrálása a web_ui-n keresztül történik
    initWebUI();
    
    server.begin();
    Serial.println("[WEB] Webszerver elindítva a 80-as porton.");
}

void loopWeb() {
    dnsServer.processNextRequest();
}
#include "connections.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LittleFS.h>

static Preferences preferences;
static WebServer server(80);

static void handleRoot() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        server.send(404, "text/plain", "A fájl nem található");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

static void handleSave() {
    if (server.hasArg("ssid")) {
        String newSsid = server.arg("ssid");
        String newPass = server.arg("password");

        preferences.begin("kaptar-cfg", false);
        preferences.putString("ssid", newSsid);
        preferences.putString("password", newPass);
        preferences.end();

        server.send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Hiányzó adatok");
    }
}

void handleConnections() {
    if (!LittleFS.begin(true)) {
        // LittleFS csatolási hiba kezelése, ha szükséges
        return;
    }

    preferences.begin("kaptar-cfg", false);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        unsigned long startAttemptTime = millis();
        
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        WiFi.softAP("Kaptarmonitor-AP", "12345678");
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();
}

void handleClientTick() {
    server.handleClient();
}   
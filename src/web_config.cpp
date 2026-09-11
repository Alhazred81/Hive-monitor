#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include "web_theme.h"

extern Preferences prefs;
extern String fullMac;

void handleConnections(AsyncWebServerRequest *request) {
    String html = htmlHead("Kapcsolatok", "2");
    
    // --- Adatok kiolvasása a memóriából ---
    prefs.begin("wifi_cfg", true);
    String currentConnectedSsid = WiFi.SSID();
    if (currentConnectedSsid.length() == 0) {
        currentConnectedSsid = prefs.getString("sta_ssid", "");
    }
    String currentRadioMode = prefs.getString("radio_mode", "espnow");
    prefs.end();

    // Dinamikus HTML attribútumok a legördülő menühöz
    String espnowSelected = (currentRadioMode == "espnow" || currentRadioMode != "lora") ? "selected" : "";
    String loraSelected = (currentRadioMode == "lora") ? "selected" : "";

    html += R"rawliteral(
    <div class="card">
        <h3>Rádió üzemmód</h3>
        <select id="radioSelect">
            <option value="espnow" )rawliteral" + espnowSelected + R"rawliteral(>ESP-NOW (Alapértelmezett)</option>
            <option value="lora" )rawliteral" + loraSelected + R"rawliteral(>868 MHz (LoRa)</option>
        </select>
        <button onclick="saveRadioMode()">Mentés és Újraindítás</button>
    </div>

    <div class="card">
        <h3>Helyi Wi-Fi (Teszteléshez)</h3>
        <label>Hálózat neve (SSID):</label>
        <div class="flex-row">
            <select id="staSsidSelect" onchange="document.getElementById('staSsid').value=this.value;" style="margin-bottom:6px;">
                <option value="">Válassz a fenti keresőből vagy írd be alább...</option>
            </select>
            <button onclick="scanWifi()">Keresés</button>
        </div>
        <input type="text" id="staSsid" value=")rawliteral" + currentConnectedSsid + R"rawliteral(" onfocus="if(this.value=='" + currentConnectedSsid + "')this.value='';" placeholder="Router SSID">
        
        <label>Jelszó:</label>
        <input type="password" id="staPass" placeholder="Jelszó">
        <button onclick="saveStaConfig()">Csatlakozás beállítása</button>
    </div>

    <div class="card">
        <h3>Saját Hálózat (AP Mód)</h3>
        <label>Kaptármonitor hálózat neve:</label>
        <input type="text" id="apSsid" placeholder="Monitor_XXXX">
        <label>Jelszó:</label>
        <input type="password" id="apPass" placeholder="Jelszó">
        <button onclick="saveApConfig()">AP beállítások mentése</button>
    </div>

    <script>
    // --- VÉDELEM: LoRa Hans-üzem figyelmeztetés ---
    document.addEventListener('DOMContentLoaded', function() {
        let radioSelect = document.getElementById('radioSelect');
        if (radioSelect) {
            radioSelect.addEventListener('change', function(e) {
                if (e.target.value === 'lora') {
                    const confirmed = confirm(
                        "FIGYELEM!\n\n" +
                        "Mielőtt átváltasz LoRa módba, feltétlenül ellenőrizd, hogy a megfelelő antenna csatlakoztatva van-e a modulhoz!\n\n" +
                        "Ha antenna nélkül indítod el az adást, a rádiómodul Hans-üzembe kerülhet (a végfok véglegesen leéghet).\n\n" +
                        "Biztosan átváltasz LoRa-ra?"
                    );
                    if (!confirmed) {
                        e.target.value = 'espnow';
                    }
                }
            });
        }
    });

    function scanWifi() {
        let select = document.getElementById('staSsidSelect');
        select.innerHTML = '<option value="">Keresés folyamatban...</option>';
        fetch('/api/scan_wifi')
        .then(response => response.json())
        .then(data => {
            select.innerHTML = '<option value="">Talált hálózatok (kattints a kiválasztáshoz)</option>';
            if(data.length === 0) { return; }
            data.forEach(net => {
                let opt = document.createElement('option');
                opt.value = net.ssid;
                opt.textContent = net.ssid + ' (' + net.rssi + ' dBm)';
                select.appendChild(opt);
            });
        }).catch(err => { console.error(err); });
    }

    function saveStaConfig() {
        let ssid = document.getElementById('staSsid').value;
        let pass = document.getElementById('staPass').value;
        if (!ssid) { alert('Kérlek, add meg a hálózat nevét!'); return; }
        fetch('/api/save_sta', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass)
        }).then(response => { if(response.ok) alert('Mentve. A kaptármonitor újraindul.'); });
    }

    function saveApConfig() {
        let ssid = document.getElementById('apSsid').value;
        let pass = document.getElementById('apPass').value;
        fetch('/api/save_ap', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass)
        }).then(response => { if(response.ok) alert('Mentve. A kaptármonitor újraindul.'); });
    }

    function saveRadioMode() {
        let mode = document.getElementById('radioSelect').value;
        fetch('/api/save_radio', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'radio=' + mode
        }).then(response => { if(response.ok) alert('Mentve. A kaptármonitor újraindul.'); });
    }
    </script>
    )rawliteral";

    html += htmlFoot();
    request->send(200, "text/html", html);
}

void handleConfigPage(AsyncWebServerRequest *request) {
    handleConnections(request);
}

void handleApiApInfo(AsyncWebServerRequest *request) {
    prefs.begin("wifi_cfg", true);
    String ssid = prefs.getString("ap_ssid", "Monitor_Def");
    prefs.end();
    String json = "{\"mac\":\"" + fullMac + "\",\"ssid\":\"" + ssid + "\"}";
    request->send(200, "application/json", json);
}

void handleApiSaveAp(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
        prefs.begin("wifi_cfg", false);
        prefs.putString("ap_ssid", request->getParam("ssid", true)->value());
        prefs.putString("ap_pass", request->getParam("pass", true)->value());
        prefs.end();
        request->send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
    } else {
        request->send(400, "text/plain", "Hiba");
    }
}

void handleApiSaveSta(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
        prefs.begin("wifi_cfg", false);
        prefs.putString("sta_ssid", request->getParam("ssid", true)->value());
        prefs.putString("sta_pass", request->getParam("pass", true)->value());
        prefs.end();
        request->send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
    } else {
        request->send(400, "text/plain", "Hiba");
    }
}

void handleApiSaveRadio(AsyncWebServerRequest *request) {
    if (request->hasParam("radio", true)) {
        prefs.begin("wifi_cfg", false);
        prefs.putString("radio_mode", request->getParam("radio", true)->value());
        prefs.end();
        request->send(200, "text/plain", "OK");
        delay(1000);
        ESP.restart();
    } else {
        request->send(400, "text/plain", "Hiba");
    }
}

void handleApiScanWifi(AsyncWebServerRequest *request) {
    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]";
    WiFi.scanDelete();
    request->send(200, "application/json", json);
}
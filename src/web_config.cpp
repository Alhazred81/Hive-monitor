#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include "web_theme.h"

extern Preferences prefs;
extern String fullMac;

void handleConfigPage(AsyncWebServerRequest *request) {
    String html = htmlHead("Beállítások", "2");
    
    html += R"rawliteral(
    <div class="card">
        <h3>Rádió üzemmód</h3>
        <select id="radioSelect">
            <option value="lora">868 MHz (LoRa)</option>
            <option value="wifi">2.4 GHz (WiFi / ESP-NOW)</option>
        </select>
        <button onclick="saveRadioMode()">Mentés és Újraindítás</button>
    </div>

    <div class="card">
        <h3>Helyi Wi-Fi (Teszteléshez)</h3>
        <label>Hálózat neve (SSID):</label>
        <div class="flex-row">
            <select id="staSsid">
                <option value="">Kattints a kereséshez...</option>
            </select>
            <button onclick="scanWifi()">Keresés</button>
        </div>
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
    function scanWifi() {
        let select = document.getElementById('staSsid');
        select.innerHTML = '<option value="">Keresés folyamatban...</option>';
        fetch('/api/scan_wifi')
        .then(response => response.json())
        .then(data => {
            select.innerHTML = '';
            if(data.length === 0) { select.innerHTML = '<option value="">Nem található hálózat</option>'; return; }
            data.forEach(net => {
                let opt = document.createElement('option');
                opt.value = net.ssid;
                opt.textContent = net.ssid + ' (' + net.rssi + ' dBm)';
                select.appendChild(opt);
            });
        }).catch(err => { select.innerHTML = '<option value="">Hiba a kereséskor</option>'; });
    }

    function saveStaConfig() {
        let ssid = document.getElementById('staSsid').value;
        let pass = document.getElementById('staPass').value;
        if (!ssid) { alert('Kérlek, válassz egy hálózatot!'); return; }
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
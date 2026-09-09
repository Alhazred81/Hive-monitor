#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "web_theme.h"
#include "web_main.h"

// Külső változók és függvények a szenzorokhoz
extern float currentTemp;
extern float currentHum;
extern float currentPres;
extern float currentZCR;
extern String getHiveStateString();

void handleRoot(AsyncWebServerRequest *request) {
    String html = htmlHead("Főoldal", "1");
    
    html += R"rawliteral(
    <div class="card wide">
        <h2>Szenzorok és Állapot</h2>
        <div class="row"><span class="k">Hőmérséklet</span><span class="v" id="val_temp">-- °C</span></div>
        <div class="row"><span class="k">Páratartalom</span><span class="v" id="val_hum">-- %</span></div>
        <div class="row"><span class="k">Légnyomás</span><span class="v" id="val_pres">-- hPa</span></div>
        <div class="row"><span class="k">ZCR (Frekvencia)</span><span class="v" id="val_zcr">-- Hz</span></div>
        <div class="row"><span class="k">Méhcsalád állapota</span><span class="v" id="val_state">--</span></div>
    </div>

    <script>
    function updateData() {
        fetch('/api/telemetry')
        .then(response => response.json())
        .then(data => {
            document.getElementById('val_temp').innerText = data.temp.toFixed(1) + ' °C';
            document.getElementById('val_hum').innerText = data.hum.toFixed(0) + ' %';
            document.getElementById('val_pres').innerText = data.pres.toFixed(0) + ' hPa';
            document.getElementById('val_zcr').innerText = data.zcr.toFixed(0) + ' Hz';
            document.getElementById('val_state').innerText = data.state_str;
        })
        .catch(err => console.error('Hiba az adatok lekérésekor:', err));
    }
    setInterval(updateData, 2000);
    updateData();
    </script>
    )rawliteral";
    
    html += htmlFoot();
    request->send(200, "text/html", html);
}

void handleApiTelemetry(AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"temp\":" + String(currentTemp) + ",";
    json += "\"hum\":" + String(currentHum) + ",";
    json += "\"pres\":" + String(currentPres) + ",";
    json += "\"zcr\":" + String(currentZCR) + ",";
    json += "\"state_str\":\"" + getHiveStateString() + "\"";
    json += "}";
    
    request->send(200, "application/json", json);
}
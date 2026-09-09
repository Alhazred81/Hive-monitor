#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "web_theme.h"
#include "sensors.h"

void handleSensors(AsyncWebServerRequest *request) {
    String html = htmlHead("Szenzorok", "3");

    html += "<div class='card wide'>"
            "<h2>Egyedi Szenzor Kezelés & Egészségi Állapot</h2>"
            "<p class='hint'>Kapcsold ki vagy be az egyes szenzorokat a csúszkával, és figyeld az egészségi állapotukat.</p>";

    auto renderSensorRow = [](const String& id, const String& name, const SensorState& st) {
        String statusHtml = "";
        if (!st.enabled) {
            statusHtml = "<span style='color:var(--txt2); font-size:12px;'>Kikapcsolva</span>";
        } else if (st.lastReadOk) {
            statusHtml = "<span class='g' style='font-size:12px; font-weight:bold;'>● Egészséges (OK)</span>";
        } else {
            statusHtml = "<span class='r' style='font-size:12px; font-weight:bold;'>● Hiba / Nincs jel</span>";
        }

        return "<div class='sens-row' style='display:flex; align-items:center; justify-content:space-between; padding:10px 0; border-bottom:1px solid var(--border);'>"
               "<div style='flex:1;'>"
               "<div style='font-weight:bold;'>" + name + "</div>"
               "<div style='font-size:11px; color:var(--txt2);'>Érték: <span id='val_" + id + "'>" + (st.value.length() ? st.value : "Mérés alatt...") + "</span></div>"
               "</div>"
               "<div style='margin:0 15px; text-align:center;'>" + statusHtml + "</div>"
               "<div>"
               "<label class='sens-toggle' style='--sens-color:" + (st.lastReadOk ? "var(--ok)" : "var(--err)") + "; margin:0;'>"
               "<input type='checkbox' " + String(st.enabled ? "checked" : "") + " onchange=\"toggleSensor('" + id + "', this.checked)\">"
               "<span class='slider'></span>"
               "</label>"
               "</div>"
               "</div>";
    };

    html += renderSensorRow("aht20", "AHT20 (Belső Hőmérséklet / Pára)", gAht20);
    html += renderSensorRow("bmp280", "BMP280 (Légnyomás)", gBmp280);
    html += renderSensorRow("sgp41", "SGP41 (VOC / NOx levegőminőség)", gSgp41);
    html += renderSensorRow("inmp441_1", "INMP441 Mikrofon #1", gInmp441_1);
    html += renderSensorRow("inmp441_2", "INMP441 Mikrofon #2", gInmp441_2);
    html += renderSensorRow("sht40", "Külső SHT40 (Hőmérséklet / Pára)", gSht40);

    html += "</div>";

    html += R"script(<script>
    function toggleSensor(key, state) {
        fetch('/senstoggle', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'key=' + encodeURIComponent(key) + '&on=' + (state ? '1' : '0')
        }).then(res => {
            if(res.ok) {
                setTimeout(() => location.reload(), 500);
            } else {
                alert('Hiba a szenzor átkapcsolásakor.');
            }
        });
    }

    function pollSensors() {
        fetch('/sensstatus')
        .then(res => res.json())
        .then(data => {
            for(let key in data) {
                let el = document.getElementById('val_' + key);
                if(el && data[key].value) {
                    el.innerText = data[key].value;
                }
            }
        }).catch(err => {});
    }
    setInterval(pollSensors, 3000);
    </script>)script";

    html += htmlFoot();
    request->send(200, "text/html", html);
}
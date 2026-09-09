#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include "web_theme.h"

extern AsyncWebServer server;
extern volatile bool isScanning;

void handleDiag(AsyncWebServerRequest *request) {
    String html = htmlHead("Diag", "4");

    html += "<div class='card wide'>"
            "<h2>I2C Busz Diagnosztika & Szkennelés</h2>"
            "<p class='hint'>Lekérdezi az I2C buszra csatlakoztatott aktív eszközök címeit (AHT20, BMP280, SGP41, SHT40).</p>"
            "<button class='sec' style='width:100%; margin-bottom:10px;' onclick='runI2cScan(this)'>🔎 I2C Eszközök Keresése</button>"
            "<div class='diag' id='i2cScanBox' style='min-height:90px; font-family:monospace; white-space:pre-wrap; background:#111; padding:10px; border-radius:4px; color:#0f0;'>Készen áll a szkennelésre...</div>"
            "</div>";

    html += R"script(<script>
    function runI2cScan(btn) {
        var orig = btn.innerText;
        btn.innerText = 'Szkennelés...';
        btn.disabled = true;
        document.getElementById('i2cScanBox').innerText = 'I2C busz vizsgálata folyamatban...';
        
        fetch('/api/i2cscan')
        .then(res => res.text())
        .then(txt => {
            document.getElementById('i2cScanBox').innerText = txt;
        }).catch(err => {
            document.getElementById('i2cScanBox').innerText = 'Hiba a szkennelés során: ' + err;
        }).finally(() => {
            btn.innerText = orig;
            btn.disabled = false;
        });
    }
    </script>)script";

    html += htmlFoot();
    request->send(200, "text/html", html);
}

void initDiagRoutes() {
    server.on("/diag", HTTP_GET, handleDiag);
    
    server.on("/api/i2cscan", HTTP_GET, [](AsyncWebServerRequest *request){
        isScanning = true; // Felfüggesztjük a háttér méréseket
        delay(50);         // Várjuk, hogy az aktuális mérés lefut

        String result = "Talált I2C címek:\n";
        byte count = 0;
        
        for (byte address = 1; address < 127; address++) {
            Wire.beginTransmission(address);
            byte error = Wire.endTransmission();
            if (error == 0) {
                result += " - Eszköz találva 0x";
                if (address < 16) result += "0";
                result += String(address, HEX);
                result += "\n";
                count++;
            }
            delay(2);
        }
        
        if (count == 0) {
            result += "Nem található I2C eszköz a buszon!";
        } else {
            result += "\nÖsszesen " + String(count) + " eszköz válaszolt.";
        }
        
        isScanning = false; // Visszaengedjük a méréseket
        request->send(200, "text/plain", result);
    });
}


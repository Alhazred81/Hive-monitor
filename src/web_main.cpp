#include "web_main.h"
#include "web_common.h"
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="hu">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kaptármonitor</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f9; }
        .header { background: #333; color: #fff; padding: 15px; text-align: center; }
        .tabs { display: flex; background: #ddd; }
        .tab { flex: 1; padding: 15px; text-align: center; cursor: pointer; font-weight: bold; }
        .tab.active { background: #fff; border-bottom: 2px solid #333; }
        .content { padding: 20px; display: none; background: #fff; }
        .content.active { display: block; }
        .card { border: 1px solid #ccc; padding: 15px; margin-bottom: 15px; border-radius: 5px; }
        .val { font-size: 1.2em; font-weight: bold; color: #0066cc; }
        
        #spectrum { background-color: #000; width: 100%; max-width: 500px; height: 180px; display: block; margin-bottom: 15px; border-radius: 4px; box-shadow: inset 0 0 10px rgba(0,0,0,0.8); }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
        .band-label { font-size: 0.85em; color: #555; }
    </style>
</head>
<body>
    <div class="header"><h2>Kaptármonitor AP</h2></div>
    <div class="tabs">
        <div class="tab active" onclick="showTab('main')">Főoldal</div>
        <div class="tab" onclick="showTab('sensors')">Szenzorok</div>
    </div>
    
    <div id="main" class="content active">
        <h3>Rendszerinformáció</h3>
        <p>A kaptármonitor Access Point módban működik.</p>
    </div>

    <div id="sensors" class="content">
        <div class="card">
            <h4>Klíma (AHT20 + BMP280)</h4>
            <p>Hőmérséklet: <span class="val" id="temp">--</span> &deg;C</p>
            <p>Páratartalom: <span class="val" id="hum">--</span> %</p>
            <p>Légnyomás: <span class="val" id="pres">--</span> hPa</p>
        </div>
        <div class="card">
            <h4>Hangspektrum vizualizáció (INMP441)</h4>
            <canvas id="spectrum" width="500" height="180"></canvas>
            <div class="grid">
                <div class="band-label">Sáv 1 (0-100Hz): <span class="val" id="b0">--</span> dB</div>
                <div class="band-label">Sáv 2 (100-200Hz): <span class="val" id="b1">--</span> dB</div>
                <div class="band-label">Sáv 3 (200-300Hz): <span class="val" id="b2">--</span> dB</div>
                <div class="band-label">Sáv 4 (300-400Hz): <span class="val" id="b3">--</span> dB</div>
                <div class="band-label">Sáv 5 (400-500Hz): <span class="val" id="b4">--</span> dB</div>
                <div class="band-label">Sáv 6 (500-1KHz): <span class="val" id="b5">--</span> dB</div>
                <div class="band-label">Sáv 7 (1K-3KHz): <span class="val" id="b6">--</span> dB</div>
                <div class="band-label">Sáv 8 (3K-8KHz): <span class="val" id="b7">--</span> dB</div>
            </div>
        </div>
    </div>

    <script>
        function showTab(id) {
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            document.querySelectorAll('.content').forEach(c => c.classList.remove('active'));
            event.target.classList.add('active');
            document.getElementById(id).classList.add('active');
        }

        const canvas = document.getElementById('spectrum');
        const ctx = canvas.getContext('2d');
        
        let targetBands = [0,0,0,0,0,0,0,0];
        let currentBands = [0,0,0,0,0,0,0,0];
        let peaks = [0,0,0,0,0,0,0,0];
        const maxDb = 120; // Megjelenítési skála maximuma dB-ben

        // Adatlekérés
        setInterval(() => {
            fetch('/api/data')
                .then(res => res.json())
                .then(data => {
                    document.getElementById('temp').innerText = data.temp.toFixed(1);
                    document.getElementById('hum').innerText = data.hum.toFixed(0);
                    document.getElementById('pres').innerText = data.pres.toFixed(0);
                    for(let i = 0; i < 8; i++) {
                        document.getElementById('b' + i).innerText = data.bands[i].toFixed(0);
                        targetBands[i] = data.bands[i];
                    }
                })
                .catch(err => console.error(err));
        }, 500);

        // Animációs ciklus 
        function animate() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            const numBands = 8;
            const barWidth = (canvas.width / numBands) - 4;
            
            // Színátmenet létrehozása
            const gradient = ctx.createLinearGradient(0, canvas.height, 0, 0);
            gradient.addColorStop(0, '#00ff00');
            gradient.addColorStop(0.6, '#ffff00');
            gradient.addColorStop(1, '#ff0000');

            for (let i = 0; i < numBands; i++) {
                // Értékek simítása
                currentBands[i] += (targetBands[i] - currentBands[i]) * 0.15;
                
                let val = Math.min(Math.max(currentBands[i], 0), maxDb);
                let barHeight = (val / maxDb) * canvas.height;
                let x = i * (canvas.width / numBands) + 2;
                let y = canvas.height - barHeight;

                // Eső csúcsérték logika
                if (barHeight >= peaks[i]) {
                    peaks[i] = barHeight;
                } else {
                    peaks[i] -= 1.2; 
                    if (peaks[i] < 0) peaks[i] = 0;
                }

                // Oszlopok rajzolása
                ctx.fillStyle = gradient;
                ctx.fillRect(x, y, barWidth, barHeight);

                // Csúcsjelző rajzolása (fehér vonal)
                ctx.fillStyle = '#ffffff';
                ctx.fillRect(x, canvas.height - peaks[i] - 2, barWidth, 2);
            }
            
            requestAnimationFrame(animate);
        }
        
        animate();
    </script>
</body>
</html>
)rawliteral";

void initWebMain() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html);
    });
}
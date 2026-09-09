#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "web_theme.h"

void handleRoot(AsyncWebServerRequest *request) {
    String html = htmlHead("Főoldal", "1");
    
    html += "<div class='card'>";
    html += "<h2>Kaptármonitor Állapot</h2>";
    html += "<p>A hálózati kapcsolat aktív. A szenzoradatok megjelenítése ide fog kerülni.</p>";
    html += "</div>";
    
    html += htmlFoot();
    request->send(200, "text/html", html);
}
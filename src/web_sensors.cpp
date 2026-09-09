#include "web_sensors.h"
#include "web_common.h"
#include "sensors.h"
#include "analyzer.h"
#include <ArduinoJson.h>

void initWebSensors() {
    server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;
        doc["temp"] = currentTemp;
        doc["hum"] = currentHum;
        doc["pres"] = currentPres;
        doc["state"] = getHiveStateString();
        
        JsonArray bandsArr = doc["bands"].to<JsonArray>();
        for (int i = 0; i < 8; i++) {
            bandsArr.add(currentBands[i]);
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
}


//storage.cpp

#include "storage.h"
#include <LittleFS.h>

void initStorage() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS hiba!");
        return;
    }
    cleanupOldLogs(14);
}

String getLogFileName(int dayOffset) {
    // Egyszerűsített sorszámozott rotáció (0-13 fájl)
    return "/log_" + String(dayOffset % 14) + ".bin";
}

void appendHiveRecord(const HiveRecord& record) {
    // Az aktuális nap sorszáma alapján írunk (pl. millis() / napi milliszekundumok alapján, vagy egyszerű index)
    int currentDayIndex = (millis() / (1000UL * 60 * 60 * 24)) % 14;
    String filename = "/log_" + String(currentDayIndex) + ".bin";

    File file = LittleFS.open(filename, FILE_APPEND);
    if (!file) {
        // Ha még nem létezik a fájl ma, létrehozzuk (és előtte esetleg töröljük a régit)
        file = LittleFS.open(filename, FILE_WRITE);
    }
    
    if (file) {
        file.write((uint8_t*)&record, sizeof(HiveRecord));
        file.close();
    }
}

void cleanupOldLogs(int maxDays) {
    // Ellenőrizzük a fájlrendszert és töröljük a felesleges vagy túl régi fájlokat
    for (int i = 0; i < 14; i++) {
        String filename = "/log_" + String(i) + ".bin";
        // Szükség esetén itt vizsgálható a fájl dátuma vagy sorszáma
    }
}
//storage.h

#pragma once
#include <Arduino.h>

struct HiveRecord {
    uint32_t timestamp;
    float temp;
    float hum;
    float pres;
    float zcr;
    uint8_t state;
    float bands[8];
};

void initStorage();
void appendHiveRecord(const HiveRecord& record);
void cleanupOldLogs(int maxDays = 14);
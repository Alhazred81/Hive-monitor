//analyzer.cpp

#include "analyzer.h"
#include "sensors.h"
#include "spec_ana.h" // Szükséges a currentBands és currentZCR eléréséhez

HiveState currentHiveState = STATE_NORMAL;

double slowEMA[8] = {0};
double fastEMA[8] = {0};

// ZCR mozgóátlagok
double slowZCR = 0;
double fastZCR = 0;

const double alphaSlow = 0.01;
const double alphaFast = 0.2;
bool firstRun = true;

void initAnalyzer() {}

void updateAnalyzer() {
    if (firstRun) {
        for (int i = 0; i < 8; i++) {
            slowEMA[i] = currentBands[i];
            fastEMA[i] = currentBands[i];
        }
        slowZCR = currentZCR;
        fastZCR = currentZCR;
        firstRun = false;
        return;
    }

    // Frekvenciasávok mozgóátlagának frissítése
    for (int i = 0; i < 8; i++) {
        slowEMA[i] = (currentBands[i] * alphaSlow) + (slowEMA[i] * (1.0 - alphaSlow));
        fastEMA[i] = (currentBands[i] * alphaFast) + (fastEMA[i] * (1.0 - alphaFast));
    }

    // ZCR mozgóátlag frissítése
    slowZCR = (currentZCR * alphaSlow) + (slowZCR * (1.0 - alphaSlow));
    fastZCR = (currentZCR * alphaFast) + (fastZCR * (1.0 - alphaFast));

    // Állapotok kiértékelése prioritás szerint
    if (motionDetected) {
        currentHiveState = STATE_STRESS;
        motionDetected = false; // Flag törlése
    }
    // Rablás detektálása: Ha a gyors ZCR átlag jelentősen meghaladja a lassút, vagy extrém magas
    else if (fastZCR > slowZCR + 40.0 || fastZCR > 200.0) {
        currentHiveState = STATE_ROBBING;
    }
    else if (fastEMA[6] > slowEMA[6] + 15.0 || fastEMA[7] > slowEMA[7] + 15.0) {
        currentHiveState = STATE_STRESS;
    }
    else if (fastEMA[3] > slowEMA[3] + 20.0 || fastEMA[4] > slowEMA[4] + 20.0) {
        currentHiveState = STATE_QUEEN_SOUND;
    }
    else if (slowEMA[2] > slowEMA[0] && slowEMA[3] > slowEMA[1]) {
        currentHiveState = STATE_QUEENLESS;
    }
    else {
        currentHiveState = STATE_NORMAL;
    }
}

String getHiveStateString() {
    switch (currentHiveState) {
        case STATE_STRESS: return "Stressz / Zargatas";
        case STATE_QUEENLESS: return "Anyatlansag gyanu";
        case STATE_QUEEN_SOUND: return "Anyahang detektalva";
        case STATE_ROBBING: return "Rablas gyanu (Magas ZCR)";
        default: return "Normal aktivitas";
    }
}


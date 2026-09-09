//analyzer.h

#pragma once
#include <Arduino.h>

enum HiveState {
    STATE_NORMAL,
    STATE_STRESS,
    STATE_QUEENLESS,
    STATE_QUEEN_SOUND,
    STATE_ROBBING // Új állapot a rablásnak
};

extern HiveState currentHiveState;
extern String getHiveStateString();

void initAnalyzer();
void updateAnalyzer();
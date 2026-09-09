//spec_ana.h

#pragma once
#include <Arduino.h>

extern double currentBands[8];
extern double currentZCR; 

void initSpecAna();
void updateSpecAna();
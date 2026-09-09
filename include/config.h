#pragma once
#include <Arduino.h>

#define I2C_SDA_PIN   5
#define I2C_SCL_PIN   6

#define I2S_WS_PIN    0
#define I2S_SCK_PIN   1
#define I2S_SD_PIN    2
#define I2S_PORT      I2S_NUM_0
#define MPU_INT_PIN 3

#define FFT_SAMPLES       512
#define FFT_SAMPLING_FREQ 16000.0

#define AP_SSID           "Kaptarmonitor_AP"
#define DEFAULT_AP_PASS   "12345678"
#define DNS_PORT_NUM      53
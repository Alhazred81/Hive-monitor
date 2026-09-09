#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "sensors.h"
#include "spec_ana.h"
#include "web_common.h"
#include "analyzer.h"

#define WAKE_PIN 3 



void setup() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0 || wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 || wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        
        if (!checkMpuSeverity()) {
            // S3-hoz való felébresztés engedélyezése, C3 esetén kihagyva
            #ifndef CONFIG_IDF_TARGET_ESP32C3
                esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
            #endif
            esp_deep_sleep_start();
        }
    }

    Serial.begin(115200);
    delay(1000);
    
    initSensors();
    initAnalyzer();
    initWeb();
    initSpecAna();

    // S3-hoz való felébresztés engedélyezése, C3 esetén kihagyva
    #ifndef CONFIG_IDF_TARGET_ESP32C3
        esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKE_PIN, 1);
    #endif
}

void loop() {
    updateSensors();
    updateAnalyzer();
    loopWeb();
    updateSpecAna();

    Serial.printf("Hőm: %.1f C | Pára: %.0f %% | Nyomás: %.0f hPa\n", currentTemp, currentHum, currentPres);
    Serial.print("Állapot: ");
    Serial.println(getHiveStateString());
    
    Serial.println("0-100\t100-200\t200-300\t300-400\t400-500\t500-1k\t1k-3k\t3k-8k\t(Hz)");
    for (int i = 0; i < 8; i++) {
        Serial.printf("%.0f\t", currentBands[i]);
    }
    Serial.println("(dB)\n------------------------------------------------------------------");

    delay(1000);
}
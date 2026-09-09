#include "sensors.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
Adafruit_MPU6050 mpu;

float currentTemp = 0.0;
float currentHum = 0.0;
float currentPres = 0.0;
volatile bool motionDetected = false;

void IRAM_ATTR handleMotionInterrupt() {
    motionDetected = true;
}

void initSensors() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (!aht.begin()) {
        Serial.println(F("AHT20 hiba"));
    }
    
    if (!bmp.begin(0x76) && !bmp.begin(0x77)) {
        Serial.println(F("BMP280 hiba"));
    }

    if (mpu.begin()) {
        mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
        mpu.setMotionDetectionThreshold(5);
        mpu.setMotionDetectionDuration(5);
        mpu.setInterruptPinLatch(true);
        mpu.setInterruptPinPolarity(true);
        mpu.setMotionInterrupt(true);

        pinMode(MPU_INT_PIN, INPUT_PULLDOWN);
        attachInterrupt(digitalPinToInterrupt(MPU_INT_PIN), handleMotionInterrupt, RISING);
    } else {
        Serial.println(F("MPU6050 hiba"));
    }
}

void updateSensors() {
    sensors_event_t humidity, temp_aht;
    aht.getEvent(&humidity, &temp_aht);
    currentTemp = temp_aht.temperature;
    currentHum = humidity.relative_humidity;
    currentPres = bmp.readPressure() / 100.0F;
}

bool checkMpuSeverity() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    if (abs(a.acceleration.x) > 3.0 || abs(a.acceleration.y) > 3.0 || abs(a.acceleration.z - 9.8) > 3.0) {
        return true; 
    }
    return false; 
}
#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

class LightSensor {
public:
    bool begin();
    float getLux();
    bool isAvailable();
    String getSensorInfo();
    void setSimulationMode(bool simulate);

private:
    BH1750 lightMeter;
    bool sensorFound = false;
    bool simulationMode = false;
    float simulatedLux = 1000.0;
    unsigned long lastRead = 0;
};

#endif

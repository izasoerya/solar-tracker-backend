#pragma once

#include <Arduino.h>
#include <Wire.h>

class SensorLDR
{
private:
    byte pin[6];
    byte value[6] = {0, 0, 0, 0, 0, 0};

public:
    SensorLDR(byte pin[6])
    {
        memcpy(this->pin, pin, sizeof(this->pin));
    }
    ~SensorLDR() {};

    void begin()
    {
        for (int i : pin)
        {
            pinMode(pin[i], INPUT);
        }
    }

    void readAll()
    {
        for (int i : pin)
        {
            value[i] = analogRead(pin[i]);
        }
    }

    byte getValue(int index)
    {
        return value[index];
    }
};

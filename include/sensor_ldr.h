
/** GENERAL DESCRIPTION
 * @brief Weighted Vector Reading to calculate light using centroid
 *
 */

#pragma once
#include <Arduino.h>
#include <Wire.h>

class SensorLDR
{
private:
    byte pin[6];
    byte value[6] = {0, 0, 0, 0, 0, 0};
    float weightedVectorX[6] = {-1, 0, 1, -1, 0, 1};
    float weightedVectorY[6] = {-1, -1, -1, 1, 1, 1};

public:
    SensorLDR(byte pin[6])
    {
        memcpy(this->pin, pin, sizeof(this->pin));
    }
    ~SensorLDR() {};

    void begin()
    {
        for (int i = 0; i < 6; i++)
        {
            pinMode(pin[i], INPUT);
        }
    }

    void update()
    {
        for (int i = 0; i < 6; i++)
        {
            value[i] = analogRead(pin[i]);
        }
    }

    uint16_t getRawValue(int index)
    {
        return value[index];
    }

    float getVectorX()
    {
        float sumX = 0;
        float totalSum = 0;
        for (int i = 0; i < 6; i++)
        {
            float val = (float)value[i];
            sumX += val * weightedVectorX[i];
            totalSum += val;
        }
        if (totalSum == 0)
            return 0;
        return sumX / totalSum;
    }

    float getVectorY()
    {
        float sumY = 0;
        float totalSum = 0;
        for (int i = 0; i < 6; i++)
        {
            float val = (float)value[i];
            sumY += val * weightedVectorY[i];
            totalSum += val;
        }
        if (totalSum == 0)
            return 0;
        return sumY / totalSum;
    }
};
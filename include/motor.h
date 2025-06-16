#ifndef SENSOR_LDR_H
#define SENSOR_LDR_H

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief SensorLDR class for reading LDR sensor values using weighted vector readings.
 *
 * This class reads analog values from an array of LDR sensors and calculates
 * normalized X and Y centroids based on weighted values.
 */
class SensorLDR
{
private:
    byte pin[6];
    byte value[6] = {0, 0, 0, 0, 0, 0};
    float weightedVectorX[6] = {-1, 0, 1, -1, 0, 1};
    float weightedVectorY[6] = {-1, -1, -1, 1, 1, 1};

public:
    /**
     * @brief Constructs a SensorLDR object.
     * @param pins An array of 6 pin numbers connected to the LDR sensors.
     */
    SensorLDR(byte pins[6]);

    /**
     * @brief Initializes the sensor pins.
     */
    void begin();

    /**
     * @brief Updates the sensor readings.
     */
    void update();

    /**
     * @brief Returns the raw analog value at the specified index.
     * @param index The sensor index.
     * @return uint16_t The raw sensor value.
     */
    uint16_t getRawValue(int index);

    /**
     * @brief Calculates the normalized X centroid using weighted values.
     * @return float The normalized X value.
     */
    float getNormalizedX();

    /**
     * @brief Calculates the normalized Y centroid using weighted values.
     * @return float The normalized Y value.
     */
    float getNormalizedY();
};

#endif // SENSOR_LDR_H

// ------------------------------
// Implementation Section
// ------------------------------

SensorLDR::SensorLDR(byte pins[6])
{
    memcpy(this->pin, pins, sizeof(this->pin));
}

void SensorLDR::begin()
{
    for (int i = 0; i < 6; i++)
    {
        pinMode(pin[i], INPUT);
    }
}

void SensorLDR::update()
{
    for (int i = 0; i < 6; i++)
    {
        value[i] = analogRead(pin[i]);
    }
}

uint16_t SensorLDR::getRawValue(int index)
{
    return value[index];
}

float SensorLDR::getNormalizedX()
{
    float sumX = 0;
    float totalSum = 0;
    for (int i = 0; i < 6; i++)
    {
        float val = static_cast<float>(value[i]);
        sumX += val * weightedVectorX[i];
        totalSum += val;
    }
    if (totalSum == 0)
        return 0;
    return sumX / totalSum;
}

float SensorLDR::getNormalizedY()
{
    float sumY = 0;
    float totalSum = 0;
    for (int i = 0; i < 6; i++)
    {
        float val = static_cast<float>(value[i]);
        sumY += val * weightedVectorY[i];
        totalSum += val;
    }
    if (totalSum == 0)
        return 0;
    return sumY / totalSum;
}
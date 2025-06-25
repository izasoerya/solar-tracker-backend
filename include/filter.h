#pragma once

#include <Arduino.h>

class LowPassFilter
{
private:
    const uint8_t alpha = 0.1;
    float prevFiltered = 1;

public:
    LowPassFilter();
    ~LowPassFilter();
    float reading(float newReading);
};

LowPassFilter::LowPassFilter() {}

LowPassFilter::~LowPassFilter() {}

float LowPassFilter::reading(float newReading)
{
    return alpha * newReading + (1 - alpha) * prevFiltered;
}

class MovingAverageFilter
{
private:
    static const uint8_t windowSize = 15;
    float buffer[windowSize];
    uint8_t index = 0;
    uint8_t count = 0;

public:
    MovingAverageFilter();
    ~MovingAverageFilter();
    float reading(float newReading);
};

MovingAverageFilter::MovingAverageFilter() {}

MovingAverageFilter::~MovingAverageFilter() {}

float MovingAverageFilter::reading(float newReading)
{
    buffer[index] = newReading;
    index = (index + 1) % windowSize;
    if (count < windowSize)
        count++;

    float sum = 0;
    for (int i = 0; i < count; i++)
        sum += buffer[index];
    return sum / count;
}

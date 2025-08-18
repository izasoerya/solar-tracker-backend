#pragma once

#include <Arduino.h>

class LowPassFilter
{
private:
    float _alpha;
    float _filteredValue;
    bool _hasInitialValue = false;

public:
    LowPassFilter() : _alpha(0.3), _filteredValue(0.0) {}
    ~LowPassFilter() {}

    float reading(float newReading)
    {
        if (!_hasInitialValue)
        {
            _filteredValue = newReading;
            _hasInitialValue = true;
        }

        float newFilteredValue = _alpha * newReading + (1.0 - _alpha) * _filteredValue;
        _filteredValue = newFilteredValue;
        return _filteredValue;
    }
};

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

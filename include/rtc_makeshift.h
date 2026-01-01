#pragma once
#include <Arduino.h>
#include "sensor_rtc.h"

class RTCMakeshift
{
private:
    timeObject simTime;
    unsigned long lastUpdate = 0;

public:
    RTCMakeshift()
    {
        simTime.hour = 6; // Start at 06:00:00
        simTime.minute = 0;
        simTime.second = 0;
        simTime.day = 20;
        simTime.month = 8;
        simTime.year = 25;
    }

    void begin()
    {
        lastUpdate = millis();
    }

    void update()
    {
        unsigned long now = millis();
        if (now - lastUpdate >= 1000)
        { // Every 1 second
            lastUpdate += 1000;
            simTime.minute += 5;
            if (simTime.minute >= 60)
            {
                simTime.minute -= 60;
                simTime.hour++;
                if (simTime.hour >= 24)
                {
                    simTime.hour = 0;
                    simTime.day++;
                    // Optionally handle month/year rollover
                }
            }
        }
    }

    timeObject getData()
    {
        return simTime;
    }
};
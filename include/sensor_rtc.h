#pragma once

#include "Arduino.h"
#include "uRTCLib.h"

struct timeObject
{
    byte hour;
    byte day;
    byte month;
};

class SensorRTC
{
private:
    timeObject tb;
    uRTCLib rtc;

public:
    void begin()
    {
        URTCLIB_WIRE.begin();

        // Comment this when done calibrating
        // rtc.set(0, 3, 21, 4, 20, 8, 25);
    }

    timeObject getData()
    {
        tb.day = rtc.minute();
        tb.hour = rtc.hour();
        tb.month = rtc.second();
        return tb;
    }

    void update() { rtc.refresh(); }
};
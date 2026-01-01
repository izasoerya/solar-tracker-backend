#pragma once

#include "Arduino.h"
#include "uRTCLib.h"

struct timeObject
{
    byte second;
    byte minute;
    byte hour;
    byte day;
    byte month;
    byte year;
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
        tb.second = rtc.second();
        tb.minute = rtc.minute();
        tb.hour = rtc.hour();
        tb.day = rtc.day();
        tb.month = rtc.month();
        tb.year = rtc.year();
        return tb;
    }

    void update() { rtc.refresh(); }
};
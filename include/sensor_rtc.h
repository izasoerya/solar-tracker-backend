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
        rtc.set(0, 42, 16, 6, 2, 5, 15);
    }

    void update() { rtc.refresh(); }
    timeObject getData() { return tb; }
};
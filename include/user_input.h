#pragma once

#include <RotaryEncoder.h>

class UserInput
{
private:
    const uint8_t pinCLK = 11;
    const uint8_t pinDT = 12;
    const uint8_t pinSW = 13;

    RotaryEncoder encoder;
    int lastDir = 0;
    bool lastButtonState = HIGH;
    bool pressDetected = false;

public:
    UserInput() : encoder(pinCLK, pinDT, RotaryEncoder::LatchMode::TWO03) {}

    void init()
    {
        pinMode(pinSW, INPUT_PULLUP);
    }

    void update()
    {
        encoder.tick();

        auto dir = encoder.getDirection();
        if (dir == RotaryEncoder::Direction::CLOCKWISE)
        {
            lastDir = 1;
        }
        else if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE)
        {
            lastDir = -1;
        }
        else
        {
            lastDir = 0;
        }

        bool btn = digitalRead(pinSW);
        if (lastButtonState == HIGH && btn == LOW)
        {
            pressDetected = true;
        }
        lastButtonState = btn;
    }

    int getDirection()
    {
        int d = lastDir;
        lastDir = 0;
        return d;
    }

    bool wasPressed()
    {
        bool was = pressDetected;
        pressDetected = false;
        return was;
    }
};

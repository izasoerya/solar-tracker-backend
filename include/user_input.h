#pragma once

#include <RotaryEncoder.h>
#include "user_interface.h"

class UserInput
{
private:
    const uint8_t pinCLK = 11;
    const uint8_t pinDT = 12;
    const uint8_t pinSW = 13;
    RotaryEncoder::Direction direction = RotaryEncoder::Direction::NOROTATION;
    RotaryEncoder encoder;

public:
    UserInput() : encoder(RotaryEncoder(pinCLK, pinDT, RotaryEncoder::LatchMode::TWO03)) {};
    ~UserInput() {};

    void read(UserInterface &ui)
    {
        encoder.tick();
        direction = encoder.getDirection();
        if (digitalRead(pinSW) == LOW)
        {
            ui.changePage();
        }
    }

    int getDirection(void *calback(int))
    {
        switch (direction)
        {
        case RotaryEncoder::Direction::NOROTATION:
            return 0;
            break;
        case RotaryEncoder::Direction::CLOCKWISE:
            return 1;
            break;
        case RotaryEncoder::Direction::COUNTERCLOCKWISE:
            return -1;
            break;
        default:
            return 0;
            break;
        }
        direction = RotaryEncoder::Direction::NOROTATION;
    }
};

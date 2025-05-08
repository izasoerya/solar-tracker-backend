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
        if (ui.currentState == UserInterfaceState::IDLE)
        {
            if (digitalRead(pinSW) == LOW)
            {
                ui.changePage();
            }
        }
        else if (ui.currentState == UserInterfaceState::MANUAL)
        {
            uint8_t x = 0, y = 0;
            bool latch = false;
            if (digitalRead(pinSW) == LOW && ui.manualState == ManualObjectState::X)
            {
                latch = true;
                if (!latch && getDirection() == 1)
                {
                    x++;
                    ui.manualDisplay(0, x, y);
                }
                else if (getDirection() == -1)
                {
                    x--;
                    ui.manualDisplay(0, x, y);
                }
                if (!digitalRead(pinSW))
                    latch = false;
            }
            else if (digitalRead(pinSW) == LOW && ui.manualState == ManualObjectState::Y)
            {
                latch = true;
                if (!latch && getDirection() == 1)
                {
                    y++;
                    ui.manualDisplay(0, x, y);
                }
                else if (getDirection() == -1)
                {
                    y--;
                    ui.manualDisplay(0, x, y);
                }
                if (!digitalRead(pinSW))
                    latch = false;
            }
            else if (digitalRead(pinSW) == LOW && ui.manualState == ManualObjectState::BACK)
            {
                latch = true;
                ui.changePage();
            }
            else
            {
                if (getDirection() == 1)
                {
                    ui.moveCursor(true);
                }
                else if (getDirection() == -1)
                {
                    ui.moveCursor(false);
                }
            }
        }
    }

    int getDirection()
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

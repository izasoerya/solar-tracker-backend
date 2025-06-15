
/** GENERAL DESCRIPTION
 * @brief Use 2 motor each to control X axis and Y axis
 * Create object motor here and assign pinout
 */

#pragma once
#include <Arduino.h>
#include "motor.h"

// TODO: CHANGE THE PINOUT LATER
Motor driverX(1, 2, 3, 4);
Motor driverY(1, 2, 3, 4);

class ControlSystem
{
private:
    const float offsetAnglePositive = 2;
    const float offsetAngleNegative = 2;
    Motor motorX = driverX;
    Motor motorY = driverY;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(float axisX, float axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
};

ControlSystem::ControlSystem() {}

ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(float axisX, float axisY, float roll, float pitch)
{
    if (axisX > 0)
    {
        if (axisX < roll + offsetAnglePositive)
        {
            motorX.turnLeft(axisX);
        }
        else
        {
            motorX.stop();
        }
    }
    else if (axisX < 0)
    {
        if (abs(axisX) < abs(roll + offsetAngleNegative))
        {
            motorX.turnRight(axisX);
        }
        else
        {
            motorX.stop();
        }
    }

    if (axisY > 0)
    {
        if (axisY < pitch + offsetAnglePositive)
        {
            motorY.turnLeft(axisY);
        }
        else
        {
            motorY.stop();
        }
    }
    else if (axisY < 0)
    {
        if (abs(axisY) < abs(pitch + offsetAngleNegative))
        {
            motorY.turnRight(axisY);
        }
        else
        {
            motorY.stop();
        }
    }
}

/** GENERAL DESCRIPTION
 * @brief Use 2 motor each to control X axis and Y axis
 * Create object motor here and assign pinout
 */

#pragma once
#include <Arduino.h>
#include "motor.h"

Motor driverX(3, 4, 5, 6);
Motor driverY(7, 8, 9, 10);

class ControlSystem
{
private:
    const float offsetAnglePositive = 10;
    const float offsetAngleNegative = 10;
    const float automaticSpeed = 5;
    Motor motorX = driverX;
    Motor motorY = driverY;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(int8_t axisX, int8_t axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
};

ControlSystem::ControlSystem() {}

ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(int8_t axisX, int8_t axisY, float roll, float pitch)
{
    // Deadband threshold to prevent oscillation
    const float deadband = 20.0;

    // X axis control (using roll)
    bool xCentered = false;
    float errorX = axisX - roll;
    if (fabs(errorX) < deadband)
    {
        motorX.stop();
        xCentered = true;
    }
    else if (errorX > 0)
    {
        motorX.turnRight(255);
    }
    else // errorX < 0
    {
        motorX.turnLeft(255);
    }

    // Only control Y if X is centered (stopped)
    // if (xCentered)
    // {
    //     float errorY = axisY - pitch;
    //     if (fabs(errorY) < deadband)
    //     {
    //         motorY.stop();
    //     }
    //     else if (errorY > 0)
    //     {
    //         motorY.turnRight(255);
    //     }
    //     else // errorY < 0
    //     {
    //         motorY.turnLeft(255);
    //     }
    // }
    // else
    // {
    //     motorY.stop();
    // }
}

void ControlSystem::runAutomatic(float centerVectorX, float centerVectorY)
{
    if (centerVectorX > 0.1)
    {
        motorX.turnLeft(automaticSpeed);
    }
    else if (centerVectorX < -0.1)
    {
        motorX.turnRight(automaticSpeed);
    }
    else
    {
        motorX.stop();
    }

    if (centerVectorY > 0.1)
    {
        motorY.turnLeft(automaticSpeed);
    }
    else if (centerVectorY < -0.1)
    {
        motorY.turnRight(automaticSpeed);
    }
    else
    {
        motorY.stop();
    }
}
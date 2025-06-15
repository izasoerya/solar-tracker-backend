
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
    Motor motorX = driverX;
    Motor motorY = driverY;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(float axisX, float axisY);
    void runAutomatic(float centerVectorX, float centerVectorY);
};

ControlSystem::ControlSystem() {}

ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(float axisX, float axisY)
{
    if (axisX > 0)
        motorX.turnLeft(axisX);
    else if (axisX < 0)
        motorX.turnRight(axisX);
    if (axisY > 0)
        motorY.turnLeft(axisY);
    else if (axisY < 0)
        motorY.turnRight(axisY);
}
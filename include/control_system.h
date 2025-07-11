/** GENERAL DESCRIPTION
 * @brief Use 2 motors to independently control X and Y axes using a state machine.
 * Create motor objects here and assign pinouts.
 */

#pragma once
#include <Arduino.h>
#include "motor.h"

Motor driverX(3, 4, 5, 6);
Motor driverY(7, 8, 9, 10);

class ControlSystem
{
private:
    enum ControlState
    {
        SEEKING,
        HOLDING
    };

    ControlState stateX = SEEKING;
    ControlState stateY = SEEKING;
    int8_t lastAxisX;
    int8_t lastAxisY;
    Motor motorX = driverX;
    Motor motorY = driverY;

    const uint8_t MAX_MOTOR_SPEED = 255;
    const float AUTOMATIC_THRESHOLD = 0.1;
    const float OFFSET_ANGLE = 1.0;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(int8_t axisX, int8_t axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
    void mockX();
    void mockY();
};

ControlSystem::ControlSystem() : stateX(SEEKING), lastAxisX(-128), stateY(SEEKING), lastAxisY(-128) {}

ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(int8_t axisX, int8_t axisY, float roll, float pitch)
{
    // --- X-Axis Control ---
    if (axisX != lastAxisX)
    {
        stateX = SEEKING;
        lastAxisX = axisX;
    }

    float errorX = axisX - roll;

    switch (stateX)
    {
    case SEEKING:
        if (fabs(errorX) < OFFSET_ANGLE)
        {
            motorX.stop();
            stateX = HOLDING;
        }
        else
        {
            if (errorX > 0)
            {
                motorX.turnRight(MAX_MOTOR_SPEED);
            }
            else
            { // errorX < 0
                motorX.turnLeft(MAX_MOTOR_SPEED);
            }
        }
        break;

    case HOLDING:
        if (fabs(errorX) > OFFSET_ANGLE)
        {
            stateX = SEEKING;
        }
        else
        {
            motorX.stop();
        }
        break;
    }

    // --- Y-Axis Control ---
    if (axisY != lastAxisY)
    {
        stateY = SEEKING;
        lastAxisY = axisY;
    }

    float errorY = axisY - pitch;

    switch (stateY)
    {
    case SEEKING:
        if (fabs(errorY) < OFFSET_ANGLE)
        {
            motorY.stop();
            stateY = HOLDING;
        }
        else
        {
            if (errorY > 0)
            {
                motorY.turnRight(MAX_MOTOR_SPEED);
            }
            else
            { // errorY < 0
                motorY.turnLeft(MAX_MOTOR_SPEED);
            }
        }
        break;

    case HOLDING:
        if (fabs(errorY) > OFFSET_ANGLE)
        {
            stateY = SEEKING;
        }
        else
        {
            motorY.stop();
        }
        break;
    }
}

void ControlSystem::runAutomatic(float centerVectorX, float centerVectorY)
{
    if (centerVectorX > AUTOMATIC_THRESHOLD)
    {
        motorX.turnLeft(255);
    }
    else if (centerVectorX < -AUTOMATIC_THRESHOLD)
    {
        motorX.turnRight(255);
    }
    else
    {
        motorX.stop();
    }

    if (centerVectorY > AUTOMATIC_THRESHOLD)
    {
        motorY.turnLeft(255);
    }
    else if (centerVectorY < -AUTOMATIC_THRESHOLD)
    {
        motorY.turnRight(255);
    }
    else
    {
        motorY.stop();
    }
}

void ControlSystem::mockY()
{
    motorY.turnLeft(255);
}
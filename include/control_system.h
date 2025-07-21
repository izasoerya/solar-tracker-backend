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
    Motor &motorX = driverX;
    Motor &motorY = driverY;

    const uint8_t MAX_MOTOR_SPEED = 150;
    const uint8_t OFFSET_ANGLE = 10;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(int8_t axisX, int8_t axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
    void mockX();
    void mockY();
    void mockXY(bool dir);
};

ControlSystem::ControlSystem() : stateX(SEEKING), lastAxisX(-128), stateY(SEEKING), lastAxisY(-128) {}

ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(int8_t axisX, int8_t axisY, float roll, float pitch)
{
    int16_t rollScaled = static_cast<int16_t>(roll * 10);
    int16_t pitchScaled = static_cast<int16_t>(pitch * 10);
    int16_t targetX = static_cast<int16_t>(axisX * 10);
    int16_t targetY = static_cast<int16_t>(axisY * 10);

    int16_t errorX = targetX - rollScaled;
    int16_t errorY = targetY - pitchScaled;

    // --- X Axis ---
    if (axisX != lastAxisX)
    {
        stateX = SEEKING;
        lastAxisX = axisX;
    }

    switch (stateX)
    {
    case SEEKING:
        if (abs(errorX) <= OFFSET_ANGLE)
        {
            motorX.stop();
            stateX = HOLDING;
        }
        else if (errorX > 0)
        {
            motorX.turnRight(MAX_MOTOR_SPEED);
        }
        else
        {
            motorX.turnLeft(MAX_MOTOR_SPEED);
        }
        break;

    case HOLDING:
        if (abs(errorX) > OFFSET_ANGLE + 5) // hysteresis
        {
            stateX = SEEKING;
        }
        else
        {
            motorX.stop();
        }
        break;
    }

    // --- Y Axis ---
    if (axisY != lastAxisY)
    {
        stateY = SEEKING;
        lastAxisY = axisY;
    }

    switch (stateY)
    {
    case SEEKING:
        if (abs(errorY) <= OFFSET_ANGLE)
        {
            motorY.stop();
            stateY = HOLDING;
        }
        else if (errorY > 0)
        {
            motorY.turnRight(MAX_MOTOR_SPEED);
        }
        else
        {
            motorY.turnLeft(MAX_MOTOR_SPEED);
        }
        break;

    case HOLDING:
        if (abs(errorY) > OFFSET_ANGLE + 5)
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
    if (centerVectorX > OFFSET_ANGLE)
    {
        motorX.turnLeft(MAX_MOTOR_SPEED);
    }
    else if (centerVectorX < -OFFSET_ANGLE)
    {
        motorX.turnRight(MAX_MOTOR_SPEED);
    }
    else
    {
        motorX.stop();
    }

    if (centerVectorY > OFFSET_ANGLE)
    {
        motorY.turnLeft(MAX_MOTOR_SPEED);
    }
    else if (centerVectorY < -OFFSET_ANGLE)
    {
        motorY.turnRight(MAX_MOTOR_SPEED);
    }
    else
    {
        motorY.stop();
    }
}

void ControlSystem::mockX()
{
    motorX.turnLeft(MAX_MOTOR_SPEED);
}

void ControlSystem::mockY()
{
    motorY.turnLeft(MAX_MOTOR_SPEED);
}

void ControlSystem::mockXY(bool dir)
{
    motorX.turnLeft(MAX_MOTOR_SPEED);
    motorY.turnLeft(MAX_MOTOR_SPEED);
}
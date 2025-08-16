#pragma once
#include <Arduino.h>
#include "motor.h"

Motor driverX(3, 4, 5, 6);
Motor driverY(7, 8, 9, 10);

class ControlSystem
{
private:
    // --- Proportional Gains ---
    const float Kp_X = 25.0;
    const float Kp_Y = 25.0;
    const float Kp_AUTO = 5.0;

    // --- State Flags for Automatic Hysteresis ---
    bool isHoldingX = true; // Start in the HOLDING state
    bool isHoldingY = true;

    Motor &motorX = driverX;
    Motor &motorY = driverY;

    // --- Motor Speed & Dead Zone Definitions ---
    const uint8_t MAX_MOTOR_SPEED = 150;
    const uint8_t MIN_MOTOR_SPEED = 75;
    const float DEAD_ZONE_DEGREES = 0.1;

    const uint8_t MAX_MOTOR_SPEED_AUTO = 150;
    const uint8_t MIN_MOTOR_SPEED_AUTO = 75;
    const float DEAD_ZONE_AUTO = 0;
    const float DEAD_ZONE_HYSTERESIS = 3; // Hysteresis value. Motor won't move again until error exceeds (DEAD_ZONE_AUTO + this value).

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(float axisX, float axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
    void stop();
};

ControlSystem::ControlSystem() {}
ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(float axisX, float axisY, float roll, float pitch)
{
    // --- X Axis Control ---
    float errorX = axisX - roll;

    if (abs(errorX) > DEAD_ZONE_DEGREES)
    {
        int motorSpeedX = static_cast<int>(Kp_X * errorX);
        motorSpeedX = constrain(motorSpeedX, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

        if (motorSpeedX > 0)
        {
            motorX.turnRight(max(motorSpeedX, MIN_MOTOR_SPEED));
        }
        else if (motorSpeedX < 0)
        {
            motorX.turnLeft(max(abs(motorSpeedX), MIN_MOTOR_SPEED));
        }
    }
    else
    {
        motorX.stop();
    }

    // --- Y Axis Control ---
    float errorY = axisY - pitch;

    if (abs(errorY) > DEAD_ZONE_DEGREES)
    {
        int motorSpeedY = static_cast<int>(Kp_Y * errorY);
        motorSpeedY = constrain(motorSpeedY, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

        if (motorSpeedY > 0)
        {
            motorY.turnRight(max(motorSpeedY, MIN_MOTOR_SPEED));
        }
        else if (motorSpeedY < 0)
        {
            motorY.turnLeft(max(abs(motorSpeedY), MIN_MOTOR_SPEED));
        }
    }
    else
    {
        motorY.stop();
    }
}

void ControlSystem::runAutomatic(float centerVectorX, float centerVectorY)
{
    float errorX = centerVectorX;
    float errorY = centerVectorY;

    if (isHoldingX)
    {
        if (abs(errorX) > (DEAD_ZONE_AUTO + DEAD_ZONE_HYSTERESIS))
        {
            isHoldingX = false;
        }
        else
        {
            motorX.stop();
        }
    }

    if (!isHoldingX)
    {
        if (abs(errorX) <= DEAD_ZONE_AUTO)
        {
            isHoldingX = true;
            motorX.stop();
        }
        else
        {
            int powerLevelX = static_cast<int>(Kp_AUTO * errorX);
            if (powerLevelX > 0)
            {
                int motorSpeedX = map(powerLevelX, 1, 255, MIN_MOTOR_SPEED_AUTO, MAX_MOTOR_SPEED_AUTO);
                motorX.turnRight(motorSpeedX);
            }
            else if (powerLevelX < 0)
            {
                int motorSpeedX = map(abs(powerLevelX), 1, 255, MIN_MOTOR_SPEED_AUTO, MAX_MOTOR_SPEED_AUTO);
                motorX.turnLeft(motorSpeedX);
            }
        }
    }

    if (isHoldingY)
    {
        if (abs(errorY) > (DEAD_ZONE_AUTO + DEAD_ZONE_HYSTERESIS))
        {
            isHoldingY = false;
        }
        else
        {
            motorY.stop();
        }
    }

    if (!isHoldingY)
    {
        if (abs(errorY) <= DEAD_ZONE_AUTO)
        {
            isHoldingY = true;
            motorY.stop();
        }
        else
        {
            int powerLevelY = static_cast<int>(Kp_AUTO * errorY);
            if (powerLevelY > 0)
            {
                int motorSpeedY = map(powerLevelY, 1, 255, MIN_MOTOR_SPEED_AUTO, MAX_MOTOR_SPEED_AUTO);
                motorY.turnRight(motorSpeedY);
            }
            else if (powerLevelY < 0)
            {
                int motorSpeedY = map(abs(powerLevelY), 1, 255, MIN_MOTOR_SPEED_AUTO, MAX_MOTOR_SPEED_AUTO);
                motorY.turnLeft(motorSpeedY);
            }
        }
    }
}

void ControlSystem::stop()
{
    motorX.stop();
    motorY.stop();
}
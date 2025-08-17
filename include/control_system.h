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
    const float DEAD_ZONE_HYSTERESIS = 3; // Hysteresis value.

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(float axisX, float axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
    void runThreshold(float valueX, float valueY, float threshold); // <-- NEW METHOD
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
            if (motorSpeedX < MIN_MOTOR_SPEED)
            {
                motorSpeedX = MIN_MOTOR_SPEED;
            }
            motorX.turnRight(motorSpeedX);
        }
        else if (motorSpeedX < 0)
        {
            if (abs(motorSpeedX) < MIN_MOTOR_SPEED)
            {
                motorSpeedX = -MIN_MOTOR_SPEED;
            }
            motorX.turnLeft(abs(motorSpeedX));
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
            if (motorSpeedY < MIN_MOTOR_SPEED)
            {
                motorSpeedY = MIN_MOTOR_SPEED;
            }
            motorY.turnRight(motorSpeedY);
        }
        else if (motorSpeedY < 0)
        {
            if (abs(motorSpeedY) < MIN_MOTOR_SPEED)
            {
                motorSpeedY = -MIN_MOTOR_SPEED;
            }
            motorY.turnLeft(abs(motorSpeedY));
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

    // --- X Axis Control ---
    if (abs(errorX) > DEAD_ZONE_DEGREES + 5)
    {
        int motorSpeedX = static_cast<int>(Kp_X * errorX);
        motorSpeedX = constrain(motorSpeedX, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

        if (motorSpeedX > 0)
        {
            if (motorSpeedX < MIN_MOTOR_SPEED)
            {
                motorSpeedX = MIN_MOTOR_SPEED;
            }
            motorX.turnRight(motorSpeedX);
        }
        else if (motorSpeedX < 0)
        {
            if (abs(motorSpeedX) < MIN_MOTOR_SPEED)
            {
                motorSpeedX = -MIN_MOTOR_SPEED;
            }
            motorX.turnLeft(abs(motorSpeedX));
        }
    }
    else
    {
        motorX.stop();
    }

    // --- Y Axis Control ---
    if (abs(errorY) > DEAD_ZONE_DEGREES + 5)
    {
        int motorSpeedY = static_cast<int>(Kp_Y * errorY);
        motorSpeedY = constrain(motorSpeedY, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

        if (motorSpeedY > 0)
        {
            if (motorSpeedY < MIN_MOTOR_SPEED)
            {
                motorSpeedY = MIN_MOTOR_SPEED;
            }
            motorY.turnRight(motorSpeedY);
        }
        else if (motorSpeedY < 0)
        {
            if (abs(motorSpeedY) < MIN_MOTOR_SPEED)
            {
                motorSpeedY = -MIN_MOTOR_SPEED;
            }
            motorY.turnLeft(abs(motorSpeedY));
        }
    }
    else
    {
        motorY.stop();
    }
}

void ControlSystem::runThreshold(float valueX, float valueY, float threshold)
{
    if (valueX > threshold)
    {
        motorX.turnLeft(MIN_MOTOR_SPEED_AUTO);
    }
    else if (valueX < -threshold)
    {
        motorX.turnRight(MIN_MOTOR_SPEED_AUTO);
    }
    else
    {
        motorX.stop();
    }

    // --- Y Axis Control ---
    if (valueY > threshold)
    {
        motorY.turnLeft(MIN_MOTOR_SPEED_AUTO);
    }
    else if (valueY < -threshold)
    {
        motorY.turnRight(MIN_MOTOR_SPEED_AUTO);
    }
    else
    {
        motorY.stop();
    }
}

void ControlSystem::stop()
{
    motorX.stop();
    motorY.stop();
}
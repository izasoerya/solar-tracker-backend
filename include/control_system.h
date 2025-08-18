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
    const float Kp_X = 25.0; // Proportional gain for X-axis motor
    const float Kp_Y = 25.0; // Proportional gain for Y-axis motor
    Motor &motorX = driverX;
    Motor &motorY = driverY;

    const uint8_t MAX_MOTOR_SPEED = 150;
    const uint8_t MIN_MOTOR_SPEED = 75;
    const float DEAD_ZONE_DEGREES = 0.1;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(float axisX, float axisY, float roll, float pitch);
    void runAutomatic(float centerVectorX, float centerVectorY);
    void runThreshold(float valueX, float valueY, float threshold);
    void mockX();
    void mockY();
    void mockXY(bool dir);
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

enum AxisState
{
    SEEKING,
    HOLDING
};
AxisState stateX = SEEKING;
AxisState stateY = SEEKING;

void ControlSystem::runAutomatic(float centerVectorX, float centerVectorY)
{
    static AxisState stateX = SEEKING;
    static AxisState stateY = SEEKING;

    const float THRESHOLD_DEGREES = 5.0; // Move until within this threshold, then apply deadband

    float errorX = centerVectorX;
    float errorY = centerVectorY;

    // --- X Axis ---
    switch (stateX)
    {
    case SEEKING:
        if (abs(errorX) <= THRESHOLD_DEGREES)
        {
            motorX.stop();
            stateX = HOLDING;
        }
        else
        {
            int motorSpeedX = static_cast<int>(Kp_X * errorX);
            motorSpeedX = constrain(motorSpeedX, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

            if (motorSpeedX > 0)
            {
                if (motorSpeedX < MIN_MOTOR_SPEED)
                    motorSpeedX = MIN_MOTOR_SPEED;
                motorX.turnRight(motorSpeedX);
            }
            else if (motorSpeedX < 0)
            {
                if (abs(motorSpeedX) < MIN_MOTOR_SPEED)
                    motorSpeedX = -MIN_MOTOR_SPEED;
                motorX.turnLeft(abs(motorSpeedX));
            }
        }
        break;
    case HOLDING:
        if (abs(errorX) > THRESHOLD_DEGREES + DEAD_ZONE_DEGREES)
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
    switch (stateY)
    {
    case SEEKING:
        if (abs(errorY) <= THRESHOLD_DEGREES)
        {
            motorY.stop();
            stateY = HOLDING;
        }
        else
        {
            int motorSpeedY = static_cast<int>(Kp_Y * errorY);
            motorSpeedY = constrain(motorSpeedY, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

            if (motorSpeedY > 0)
            {
                if (motorSpeedY < MIN_MOTOR_SPEED)
                    motorSpeedY = MIN_MOTOR_SPEED;
                motorY.turnRight(motorSpeedY);
            }
            else if (motorSpeedY < 0)
            {
                if (abs(motorSpeedY) < MIN_MOTOR_SPEED)
                    motorSpeedY = -MIN_MOTOR_SPEED;
                motorY.turnLeft(abs(motorSpeedY));
            }
        }
        break;
    case HOLDING:
        if (abs(errorY) > THRESHOLD_DEGREES + DEAD_ZONE_DEGREES)
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

void ControlSystem::runThreshold(float valueX, float valueY, float threshold)
{
    // --- X Axis Control ---
    if (valueX > threshold)
    {
        // Value is too high, move left (or whichever direction is negative)
        motorX.turnLeft(120);
    }
    else if (valueX < -threshold)
    {
        // Value is too low, move right (or whichever direction is positive)
        motorX.turnRight(120);
    }
    else
    {
        // Value is within the acceptable range, stop the motor.
        motorX.stop();
    }

    // --- Y Axis Control ---
    if (valueY > threshold)
    {
        // Value is too high, move down (or whichever direction is negative)
        motorY.turnLeft(120);
    }
    else if (valueY < -threshold)
    {
        // Value is too low, move up (or whichever direction is positive)
        motorY.turnRight(120);
    }
    else
    {
        // Value is within the acceptable range, stop the motor.
        motorY.stop();
    }
}

void ControlSystem::stop()
{
    motorX.stop();
    motorY.stop();
}

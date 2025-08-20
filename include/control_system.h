/** GENERAL DESCRIPTION
 * @brief Use 2 motors to independently control X and Y axes using a state machine.
 * Create motor objects here and assign pinouts.
 */

#pragma once
#include <Arduino.h>
#include "motor.h"

Motor driverX(3, 4, 5, 6);
Motor driverY(7, 8, 9, 10);
enum AxisState
{
    SEEKING,
    HOLDING
};
AxisState stateX = SEEKING;
AxisState stateY = SEEKING;

class ControlSystem
{
private:
    const float Kp_X = 5.0; // Proportional gain for X-axis motor
    const float Kp_Y = 2.5; // Proportional gain for Y-axis motor

    Motor &motorX = driverX;
    Motor &motorY = driverY;

    const uint8_t MAX_MOTOR_SPEED = 150;
    const uint8_t MIN_MOTOR_SPEED = 75;
    const float DEAD_ZONE_DEGREES = 0.1;

    AxisState manualStateX = SEEKING;
    AxisState manualStateY = SEEKING;
    float lastTargetX = 0;
    float lastTargetY = 0;

public:
    ControlSystem();
    ~ControlSystem();

    void runManual(float axisX, float axisY, float angleMain, float angleSecond);
    void runAutomatic(float centerVectorX, float centerVectorY);
    void runThreshold(float valueX, float valueY, float threshold);
    void runRuleBased(int top, int bottom, int left, int right);
    bool runFallBackStrategy(byte seeker1, byte seeker2, byte refLeft, byte refRight);
    void cloudyStrategy(uint32_t nowMilis, byte seeker1, byte seeker2, byte refLeft, byte refRight, byte currentLeft, byte currentRight);
    void mockX();
    void mockY();
    void mockXY(bool dir);
    void stop();
};

ControlSystem::ControlSystem() {}

ControlSystem::~ControlSystem() {}

void ControlSystem::runManual(float axisX, float axisY, float angleMain, float angleSecond)
{
    float errorX = axisX - angleMain;
    float errorY = axisY - angleSecond;

    // Reset state if target changes
    if (axisX != lastTargetX)
    {
        manualStateX = SEEKING;
        lastTargetX = axisX;
    }
    if (axisY != lastTargetY)
    {
        manualStateY = SEEKING;
        lastTargetY = axisY;
    }

    // --- X Axis State Machine ---
    switch (manualStateX)
    {
    case SEEKING:
        if (abs(errorX) <= DEAD_ZONE_DEGREES)
        {
            motorX.stop();
            manualStateX = HOLDING;
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
        if (abs(errorX) > DEAD_ZONE_DEGREES + 0.2)
        {
            manualStateX = SEEKING;
        }
        else
        {
            motorX.stop();
        }
        break;
    }

    // --- Y Axis State Machine ---
    switch (manualStateY)
    {
    case SEEKING:
        if (abs(errorY) <= DEAD_ZONE_DEGREES)
        {
            motorY.stop();
            manualStateY = HOLDING;
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
        if (abs(errorY) > DEAD_ZONE_DEGREES + 0.2)
        {
            manualStateY = SEEKING;
        }
        else
        {
            motorY.stop();
        }
        break;
    }
}

void ControlSystem::runAutomatic(float diffMain, float diffSecond)
{
    static AxisState stateX = SEEKING;
    static AxisState stateY = SEEKING;

    const float Kp_AX = 15.0; // Proportional gain for X-axis motor
    const float Kp_AY = 7.5;  // Proportional gain for Y-axis motor
    const float THRESHOLD_DEGREES = 8.0;
    const float DEAD_BAND = 12.0;

    int controlX = static_cast<int>(Kp_AX * diffMain);
    int controlY = static_cast<int>(Kp_AY * diffSecond);

    controlX = constrain(controlX, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    controlY = constrain(controlY, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

    // --- X Axis ---
    switch (stateX)
    {
    case SEEKING:
        if (abs(diffMain) <= THRESHOLD_DEGREES)
        {
            motorX.stop();
            stateX = HOLDING;
        }
        else
        {
            if (controlX > 0)
            {
                if (controlX < MIN_MOTOR_SPEED)
                    controlX = MIN_MOTOR_SPEED;
                motorX.turnRight(controlX);
            }
            else if (controlX < 0)
            {
                if (abs(controlX) < MIN_MOTOR_SPEED)
                    controlX = -MIN_MOTOR_SPEED;
                motorX.turnLeft(abs(controlX));
            }
        }
        break;

    case HOLDING:
        if (abs(diffMain) > DEAD_BAND)
            stateX = SEEKING;
        else
            motorX.stop();
        break;
    }

    // --- Y Axis ---
    switch (stateY)
    {
    case SEEKING:
        if (abs(diffSecond) <= THRESHOLD_DEGREES)
        {
            motorY.stop();
            stateY = HOLDING;
        }
        else
        {
            if (controlY > 0)
            {
                if (controlY < MIN_MOTOR_SPEED)
                    controlY = MIN_MOTOR_SPEED;
                motorY.turnRight(controlY);
            }
            else if (controlY < 0)
            {
                if (abs(controlY) < MIN_MOTOR_SPEED)
                    controlY = -MIN_MOTOR_SPEED;
                motorY.turnLeft(abs(controlY));
            }
        }
        break;

    case HOLDING:
        if (abs(diffSecond) > DEAD_BAND)
            stateY = SEEKING;
        else
            motorY.stop();
        break;
    }
}

void ControlSystem::cloudyStrategy(uint32_t nowMilis, byte seeker1, byte seeker2, byte refLeft, byte refRight, byte currentLeft, byte currentRight)
{
    static uint32_t cloudyStartTime = 0;
    static bool inCloudyFallback = false;

    if (cloudyStartTime == 0)
    {
        cloudyStartTime = nowMilis;
    }
    if ((nowMilis - cloudyStartTime) >= 30000)
        inCloudyFallback = true;
    stop(); // Stop the motor

    if (inCloudyFallback)
    {
        if (seeker1 > 120 || seeker2 > 120)
        {
            bool approved = runFallBackStrategy(currentLeft, currentRight, refLeft, refRight);
            if (approved)
            {
                inCloudyFallback = false;
                cloudyStartTime = 0;
            }
        }
        else
        {
            stop(); // Stop the motor
        }
    }
}

void ControlSystem::runThreshold(float valueX, float valueY, float threshold)
{
    // --- X Axis Control ---
    if (valueX > threshold)
    {
        motorX.turnLeft(120);
    }
    else if (valueX < -threshold)
    {
        motorX.turnRight(120);
    }
    else
    {
        motorX.stop();
    }

    // --- Y Axis Control ---
    if (valueY > threshold)
    {
        motorY.turnLeft(120);
    }
    else if (valueY < -threshold)
    {
        motorY.turnRight(120);
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

void ControlSystem::runRuleBased(int top, int bottom, int left, int right)
{
    const int THRESHOLD = 100; // contoh, sesuaikan dengan kondisi cahaya
    const int SPEED = 80;      // kecepatan default gerak

    // 1. Tentukan status ON/OFF untuk tiap LDR
    bool topOn = (top > THRESHOLD);
    bool bottomOn = (bottom > THRESHOLD);
    bool leftOn = (left > THRESHOLD);
    bool rightOn = (right > THRESHOLD);

    int activeCount = topOn + bottomOn + leftOn + rightOn;

    // 2. Rule berdasarkan jumlah aktif
    if (activeCount == 1)
    {
        // === CASE 1 LDR ===
        if (topOn)
        {
            motorY.turnRight(SPEED); // ke arah atas
            motorX.stop();
        }
        else if (bottomOn)
        {
            motorY.turnLeft(SPEED); // ke arah bawah
            motorX.stop();
        }
        else if (leftOn)
        {
            motorX.turnLeft(SPEED); // ke arah kiri
            motorY.stop();
        }
        else if (rightOn)
        {
            motorX.turnRight(SPEED); // ke arah kanan
            motorY.stop();
        }
    }
    else if (activeCount == 2)
    {
        // === CASE 2 LDR ===
        // horizontal pair
        if (leftOn && rightOn)
        {
            // berarti posisi benar, stop vertical, fokus horizontal
            motorX.stop();
            // bisa kasih fine adjust kalau kiri lebih terang
        }
        // vertical pair
        else if (topOn && bottomOn)
        {
            motorY.stop();
        }
        // diagonal pair → treat as diagonal move
        else if (topOn && leftOn)
        {
            motorX.turnLeft(SPEED);
            motorY.turnRight(SPEED);
        }
        else if (topOn && rightOn)
        {
            motorX.turnRight(SPEED);
            motorY.turnRight(SPEED);
        }
        else if (bottomOn && leftOn)
        {
            motorX.turnLeft(SPEED);
            motorY.turnLeft(SPEED);
        }
        else if (bottomOn && rightOn)
        {
            motorX.turnRight(SPEED);
            motorY.turnLeft(SPEED);
        }
    }
    else if (activeCount == 3)
    {
        // === CASE 3 LDR ===
        // Yang mati menunjukkan arah berlawanan
        if (!topOn)
        {
            // top gelap → berarti harus ke bawah
            motorY.turnLeft(SPEED);
        }
        else if (!bottomOn)
        {
            motorY.turnRight(SPEED);
        }
        else if (!leftOn)
        {
            motorX.turnRight(SPEED);
        }
        else if (!rightOn)
        {
            motorX.turnLeft(SPEED);
        }
    }
    else
    {
        // === CASE 0 or 4 ===
        motorX.stop();
        motorY.stop();
    }
}

bool ControlSystem::runFallBackStrategy(byte currentLeft, byte currentRight, byte refLeft, byte refRight)
{
    const int THRESHOLD = 30;

    bool current1Close = abs((int)currentLeft - (int)refLeft) <= THRESHOLD;
    bool current2Close = abs((int)currentRight - (int)refRight) <= THRESHOLD;

    if (current1Close && current2Close)
    {
        motorX.stop();
        return true;
    }

    if (!current1Close)
    {
        if (currentLeft > refLeft)
            motorX.turnRight(MAX_MOTOR_SPEED);
        else
            motorX.turnLeft(MAX_MOTOR_SPEED);
    }
    else if (!current2Close)
    {
        if (currentRight > refRight)
            motorX.turnRight(MAX_MOTOR_SPEED / 1.5);
        else
            motorX.turnLeft(MAX_MOTOR_SPEED / 1.5);
    }
    return false;
}
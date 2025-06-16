
/** GENERAL DESCRIPTION
 * @brief Assume to use Driver BTS7960 for control motor
 * PIN is defined at the implementation
 */

#pragma once
#include <Arduino.h>

class Motor
{
private:
    byte LEN;
    byte REN;
    byte LPWM;
    byte RPWM;

public:
    /**
     * @brief Construct a new BTS7960 Driver Motor object
     *
     * @param REN Enable pin for right PWM
     * @param LEN Enable pin for left PWM
     * @param RPWM PWM pin for control PWM right
     * @param LPWM PWM pin for control PWM left
     */
    Motor(byte REN, byte LEN, byte RPWM, byte LPWM)
    {
        this->LEN = LEN;
        this->REN = REN;
        this->LPWM = LPWM;
        this->RPWM = RPWM;
        pinMode(LEN, OUTPUT);
        pinMode(REN, OUTPUT);
        digitalWrite(LEN, LOW);
        digitalWrite(REN, LOW);
    };

    ~Motor() {};

    /**
     * @brief Turns the motor to the left with the specified speed.
     *
     * @param speed The speed at which to run the motor (0 - 255).
     */
    void turnLeft(byte speed)
    {
        analogWrite(LPWM, 0);
        digitalWrite(LPWM, LOW);
        analogWrite(RPWM, speed);
    }

    /**
     * @brief Turns the motor to the right with the specified speed.
     *
     * @param speed The speed at which to run the motor (0 - 255).
     */
    void turnRight(byte speed)
    {
        analogWrite(RPWM, 0);
        digitalWrite(RPWM, LOW);
        analogWrite(LPWM, speed);
    }

    /**
     * @brief Stops the motor.
     */
    void stop()
    {
        analogWrite(RPWM, 0);
        digitalWrite(RPWM, LOW);
        analogWrite(LPWM, 0);
        digitalWrite(LPWM, 0);
    }
};
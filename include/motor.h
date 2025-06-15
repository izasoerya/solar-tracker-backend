
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

    void turnLeft(byte speed)
    {
        analogWrite(LPWM, 0);
        digitalWrite(LPWM, LOW);
        analogWrite(RPWM, speed);
    }

    void turnRight(byte speed)
    {
        analogWrite(RPWM, 0);
        digitalWrite(RPWM, LOW);
        analogWrite(LPWM, speed);
    }
};
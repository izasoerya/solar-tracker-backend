#pragma once

#include <Arduino.h>

class Motor
{
private:
    byte LEN;  // Left Enable Pin
    byte REN;  // Right Enable Pin
    byte LPWM; // Left PWM Pin
    byte RPWM; // Right PWM Pin

public:
    Motor(byte REN, byte LEN, byte RPWM, byte LPWM)
    {
        this->LEN = LEN;
        this->REN = REN;
        this->LPWM = LPWM;
        this->RPWM = RPWM;
        pinMode(LEN, OUTPUT);
        pinMode(REN, OUTPUT);
        digitalWrite(LEN, HIGH);
        digitalWrite(REN, HIGH);
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
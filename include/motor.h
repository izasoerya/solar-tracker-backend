#pragma once
#include <Arduino.h>

/**
 * @brief Class representing a BTS7960 motor driver controller.
 */
class Motor
{
public:
    /**
     * @brief Construct a new Motor object using BTS7960 driver.
     *
     * @param REN Enable pin for right PWM
     * @param LEN Enable pin for left PWM
     * @param RPWM PWM pin for control PWM right
     * @param LPWM PWM pin for control PWM left
     */
    Motor(byte REN, byte LEN, byte RPWM, byte LPWM);

    /**
     * @brief Destroy the Motor object
     */
    ~Motor();

    /**
     * @brief Turns the motor to the left with the specified speed.
     *
     * @param speed The speed (0–255).
     */
    void turnLeft(byte speed);

    /**
     * @brief Turns the motor to the right with the specified speed.
     *
     * @param speed The speed (0–255).
     */
    void turnRight(byte speed);

    /**
     * @brief Stops the motor immediately.
     */
    void stop();

private:
    byte LEN;
    byte REN;
    byte LPWM;
    byte RPWM;
};

// ------------------------------
// Implementation Section
// ------------------------------

Motor::Motor(byte REN, byte LEN, byte RPWM, byte LPWM)
{
    this->LEN = LEN;
    this->REN = REN;
    this->LPWM = LPWM;
    this->RPWM = RPWM;

    pinMode(this->LEN, OUTPUT);
    pinMode(this->REN, OUTPUT);
    pinMode(this->LPWM, OUTPUT);
    pinMode(this->RPWM, OUTPUT);

    digitalWrite(this->LEN, LOW);
    digitalWrite(this->REN, LOW);
}

Motor::~Motor() {}

void Motor::turnLeft(byte speed)
{
    analogWrite(LPWM, 0);
    digitalWrite(LPWM, LOW);
    analogWrite(RPWM, speed);
}

void Motor::turnRight(byte speed)
{
    analogWrite(RPWM, 0);
    digitalWrite(RPWM, LOW);
    analogWrite(LPWM, speed);
}

void Motor::stop()
{
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
    digitalWrite(RPWM, LOW);
    digitalWrite(LPWM, LOW);
}

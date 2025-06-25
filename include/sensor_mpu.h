#pragma once

#include <Arduino.h>
#include <Wire.h>

class SensorMPU
{
private:
    const uint8_t MPU_ADDR = 0x68;
    const uint8_t MPU_PWR_MGMT_1 = 0x6B;
    const uint8_t MPU_ACCEL_XOUT_H = 0x3B;
    const uint8_t byteSize = 6;
    float xa, ya, za, roll, pitch;

public:
    SensorMPU() {}
    ~SensorMPU() {}

    void begin()
    {
        Wire.begin();
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(MPU_PWR_MGMT_1); // reset the device
        Wire.write(0);
        Wire.endTransmission(true);
    }

    void update()
    {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(MPU_ACCEL_XOUT_H); // starting register address for accelerometer
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_ADDR, byteSize); // request 6 bytes of accelerometer data

        int t = Wire.read();
        xa = (t << 8) | Wire.read();
        t = Wire.read();
        ya = (t << 8) | Wire.read();
        t = Wire.read();
        za = (t << 8) | Wire.read();

        roll = atan2(ya, za) * 180.0 / PI;
        pitch = atan2(-xa, sqrt(ya * ya + za * za)) * 180.0 / PI;
    }

    float getRoll() const { return roll; }
    float getPitch() const { return pitch; }

    // Set accelerometer sensitivity (0=±2g, 1=±4g, 2=±8g, 3=±16g)
    void setAccelSensitivity(uint8_t level)
    {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x1C); // ACCEL_CONFIG register
        Wire.write((level & 0x03) << 3);
        Wire.endTransmission(true);
    }

    // Set gyroscope sensitivity (0=±250°/s, 1=±500°/s, 2=±1000°/s, 3=±2000°/s)
    void setGyroSensitivity(uint8_t level)
    {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x1B); // GYRO_CONFIG register
        Wire.write((level & 0x03) << 3);
        Wire.endTransmission(true);
    }
};
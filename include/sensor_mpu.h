#pragma once

#include <Arduino.h>
#include <Wire.h>

struct ModelIMU
{
    float xa;
    float ya;
    float za;
    float xg;
    float yg;
    float zg;
    float Accelroll;
    float Accelpitch;
    float Accelyaw;
};

class SensorMPU
{
private:
    const uint8_t MPU_ADDR = 0x68;
    const uint8_t MPU_PWR_MGMT_1 = 0x6B;
    const uint8_t MPU_ACCEL_XOUT_H = 0x3B;
    const uint8_t byteSize = 6;
    ModelIMU imuData;

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
        Wire.write(MPU_ACCEL_XOUT_H);
        if (Wire.endTransmission(false) != 0)
        {
            return;
        }

        if (Wire.requestFrom(MPU_ADDR, byteSize) != byteSize)
        {
            return;
        }

        int16_t rawXa = (Wire.read() << 8) | Wire.read();
        int16_t rawYa = (Wire.read() << 8) | Wire.read();
        int16_t rawZa = (Wire.read() << 8) | Wire.read();

        imuData.xa = rawXa;
        imuData.ya = rawYa;
        imuData.za = rawZa;

        imuData.Accelroll = atan2(imuData.ya, imuData.za) * 180.0 / PI;
        imuData.Accelpitch = atan2(-imuData.xa, sqrt(imuData.ya * imuData.ya + imuData.za * imuData.za)) * 180.0 / PI;
    }

    float getAccelX() const { return imuData.xa; }
    float getAccelY() const { return imuData.ya; }
    float getAccelZ() const { return imuData.za; }
    float getAccelRoll() const { return imuData.Accelroll; }
    float getAccelPitch() const { return imuData.Accelpitch; }
    ModelIMU getModelIMU() const { return imuData; }

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

    // Set filter bandwidth (1-6 = 5Hz, 10Hz, 21Hz, 44Hz, 94Hz, 184Hz, 260Hz)
    void setFilterBandwidth(uint8_t level)
    {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x1A);         // The CONFIG register (0x1A)
        Wire.write(level & 0x07); // Write the level (0-6), masking to ensure it's 3 bits
        Wire.endTransmission(true);
    }
};
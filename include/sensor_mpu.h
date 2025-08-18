#pragma once

#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN A4
#define SCL_PIN A5

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
    bool active = false;
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
        active = true;
    }

    void update()
    {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(MPU_ACCEL_XOUT_H);
        if (Wire.endTransmission(false) != 0)
        {
            // if (!i2cRecover())
            // {
            //     return; // recovery failed, let WDT handle it
            // }
            return;
        }

        uint8_t count = Wire.requestFrom(MPU_ADDR, byteSize, (bool)1);
        if (count != byteSize)
        {
            // i2cRecover(); // try to free bus
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

    bool i2cRecover()
    {
        pinMode(SCL_PIN, OUTPUT);
        pinMode(SDA_PIN, INPUT_PULLUP);

        // Toggle clock up to 9 times to free SDA
        for (uint8_t i = 0; i < 9; i++)
        {
            if (digitalRead(SDA_PIN) == HIGH)
            {
                break; // SDA released → bus is free
            }
            digitalWrite(SCL_PIN, LOW);
            delayMicroseconds(5);
            digitalWrite(SCL_PIN, HIGH);
            delayMicroseconds(5);
        }

        // Generate STOP condition
        pinMode(SDA_PIN, OUTPUT);
        digitalWrite(SDA_PIN, LOW);
        delayMicroseconds(5);
        digitalWrite(SCL_PIN, HIGH);
        delayMicroseconds(5);
        digitalWrite(SDA_PIN, HIGH);
        delayMicroseconds(5);

        // Reinit I2C
        Wire.end();
        Wire.begin();
        Wire.setWireTimeout(100000, true); // 25ms timeout, auto clear bus

        // Check if SDA is released
        return (digitalRead(SDA_PIN) == HIGH);
    }

    bool isActive() { return active; }

    void end()
    {
        active = false;
    }
};

#include <Adafruit_FXOS8700.h>

class SensorFXOSFXAS
{
private:
    Adafruit_FXOS8700 fxos = Adafruit_FXOS8700(0x8700A, 0x8700B);
    bool active = false;
    ModelIMU imuData;

public:
    SensorFXOSFXAS() {}
    ~SensorFXOSFXAS() {}

    void begin()
    {
        if (!fxos.begin())
        {
            active = false;
            return;
        }
        active = true;
    }

    void update()
    {
        if (!active)
            return;

        sensors_event_t aevent, mevent, gevent;
        fxos.getEvent(&aevent, &mevent);

        imuData.xa = aevent.acceleration.x;
        imuData.ya = aevent.acceleration.y;
        imuData.za = aevent.acceleration.z;
        imuData.xg = gevent.gyro.x;
        imuData.yg = gevent.gyro.y;
        imuData.zg = gevent.gyro.z;

        imuData.Accelroll = atan2(imuData.ya, imuData.za) * 180.0 / PI;
        imuData.Accelpitch = atan2(-imuData.xa, sqrt(imuData.ya * imuData.ya + imuData.za * imuData.za)) * 180.0 / PI;
    }

    float getAccelX() const { return imuData.xa; }
    float getAccelY() const { return imuData.ya; }
    float getAccelZ() const { return imuData.za; }
    float getGyroX() const { return imuData.xg; }
    float getGyroY() const { return imuData.yg; }
    float getGyroZ() const { return imuData.zg; }
    float getAccelRoll() const { return imuData.Accelroll; }
    float getAccelPitch() const { return imuData.Accelpitch; }
    ModelIMU getModelIMU() const { return imuData; }

    // Dummy functions for compatibility (not supported by Adafruit lib)
    void setAccelSensitivity(uint8_t level) {}
    void setGyroSensitivity(uint8_t level) {}
    void setFilterBandwidth(uint8_t level) {}

    bool isActive() { return active; }

    void end()
    {
        active = false;
    }
};

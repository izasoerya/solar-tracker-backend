#pragma once

#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN A4
#define SCL_PIN A5

struct ModelIMU
{
    float xa = 0;
    float ya = 0;
    float za = 0;
    float xg = 0;
    float yg = 0;
    float zg = 0;
    float Accelroll = 0;
    float Accelpitch = 0;
    float Accelyaw = 0;
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

    bool safeReadIMU(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len, uint16_t timeoutMs)
    {
        uint32_t start = millis();

        Wire.beginTransmission(addr);
        Wire.write(reg);
        if (Wire.endTransmission(false) != 0)
        {
            return false; // write failed
        }

        Wire.requestFrom(addr, len, (uint8_t)true);
        uint8_t i = 0;
        while (i < len)
        {
            if (Wire.available())
            {
                data[i++] = Wire.read();
            }
            if (millis() - start > timeoutMs)
            {
                return false; // timeout only applies here
            }
        }
        return true;
    }

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
        uint8_t buffer[6];

        if (!safeReadIMU(MPU_ADDR, MPU_ACCEL_XOUT_H, buffer, byteSize, 20)) // 20ms timeout only for IMU
        {
            // i2cRecover();
            return;
        }

        int16_t rawXa = (buffer[0] << 8) | buffer[1];
        int16_t rawYa = (buffer[2] << 8) | buffer[3];
        int16_t rawZa = (buffer[4] << 8) | buffer[5];

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

        sensors_event_t aevent, mevent;
        fxos.getEvent(&aevent, &mevent);

        // Basic sanity check (NaN or out-of-range values)
        if (isnan(aevent.acceleration.x) || fabs(aevent.acceleration.x) > 100.0 ||
            isnan(aevent.acceleration.y) || fabs(aevent.acceleration.y) > 100.0 ||
            isnan(aevent.acceleration.z) || fabs(aevent.acceleration.z) > 100.0)
        {
            // Timeout or invalid read – try I2C recovery
            Wire.end();
            delay(10);
            Wire.begin();
            return;
        }

        imuData.xa = -1 * aevent.acceleration.y;
        imuData.ya = aevent.acceleration.x;
        imuData.za = aevent.acceleration.z;

        // NOTE: FXOS8700 is only accel + mag, no gyro.
        imuData.xg = 0;
        imuData.yg = 0;
        imuData.zg = 0;

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

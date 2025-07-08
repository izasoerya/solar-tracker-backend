#pragma once

#include <MadgwickAHRS.h>
#include "sensor_mpu.h"
#include <Arduino.h>

class MadgwickIMU
{
private:
    unsigned long _microsPerReading;
    unsigned long _microsPrevious;
    float _roll, _pitch, _yaw;
    Madgwick _filter;
    ModelIMU _imuData;

    float convertRawAcceleration(int aRaw)
    {
        float a = (aRaw * 8.0) / 32768.0;
        return a;
    }

    float convertRawGyro(int gRaw)
    {
        float g = (gRaw * 250.0) / 32768.0;
        return g;
    }

public:
    MadgwickIMU();
    ~MadgwickIMU();

    void update(ModelIMU imu);
    float getRoll();
    float getPitch();
};

MadgwickIMU::MadgwickIMU() {}
MadgwickIMU::~MadgwickIMU() {}

void MadgwickIMU::update(ModelIMU imu)
{
    if (micros() - _microsPrevious >= _microsPerReading)
    {
        _imuData.xa = convertRawAcceleration(imu.xa);
        _imuData.ya = convertRawAcceleration(imu.ya);
        _imuData.za = convertRawAcceleration(imu.za);
        _imuData.xg = convertRawGyro(imu.xg);
        _imuData.yg = convertRawGyro(imu.yg);
        _imuData.zg = convertRawGyro(imu.zg);

        // update the filter, which computes orientation
        _filter.updateIMU(
            _imuData.xg, _imuData.yg, _imuData.zg,
            _imuData.xa, _imuData.ya, _imuData.za);

        // print the heading, pitch and roll
        _roll = _filter.getRoll();
        _pitch = _filter.getPitch();
        _yaw = _filter.getYaw();

        // increment previous time, so we keep proper pace
        _microsPrevious = _microsPrevious + _microsPerReading;
    }
}

float MadgwickIMU::getRoll() { return _roll; }

float MadgwickIMU::getPitch() { return _pitch; }
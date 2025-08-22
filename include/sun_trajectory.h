#include <Arduino.h>
#include <sensor_rtc.h>
#include <math.h>

static inline float deg2rad(float d) { return d * (M_PI / 180.0f); }
static inline float rad2deg(float r) { return r * (180.0f / M_PI); }

struct SeptyanJaya
{
    float parsedX;
    float parsedY;
};

class SunTracker
{
public:
    SunTracker();
    void update(const timeObject &time);
    SeptyanJaya septyanUpdate(float azimuth, float elevation);
    float getAzimuth() const;
    float getElevation() const;
    int calculateDayOfYear(byte day, byte month, byte year);

private:
    const float _latitude = -7.7657162;
    const float _longitude = 110.3702127;
    const float _timezone = 7;
    float _azimuth = 0;
    float _elevation = 0;

    void calculateSunAngles(int dayOfYear, float fractionalHour);
};

SunTracker::SunTracker() {}

void SunTracker::update(const timeObject &time)
{
    int dayOfYear = calculateDayOfYear(time.day, time.month, time.year);
    float fractionalHour = time.hour + time.minute / 60.0 + time.second / 3600.0;
    calculateSunAngles(dayOfYear, fractionalHour);
}

SeptyanJaya SunTracker::septyanUpdate(float azimuth, float elevation)
{
    float radX = -atan2(sin(deg2rad(elevation)), (sin(deg2rad(azimuth)) * cos(deg2rad(elevation))));
    float radY = -asin(cos(deg2rad(azimuth)) * cos(deg2rad(elevation)));
    SeptyanJaya septy;
    septy.parsedX = -90 - rad2deg(radX);
    septy.parsedY = rad2deg(radY);
    return septy;
}

float SunTracker::getAzimuth() const
{
    return _azimuth;
}

float SunTracker::getElevation() const
{
    return _elevation;
}

int SunTracker::calculateDayOfYear(byte day, byte month, byte year)
{
    byte daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint16_t doy = 0;
    uint16_t correctYear = 2000 + year;
    if (correctYear % 4 == 0 && (correctYear % 100 != 0 || correctYear % 400 == 0))
    {
        daysInMonth[2] = 29;
    }
    for (byte i = 1; i < month; ++i)
    {
        doy += daysInMonth[i];
    }
    doy += day;
    return doy;
}

void SunTracker::calculateSunAngles(int dayOfYear, float fractionalHour)
{
    // Convert latitude to radians for calculations
    float latRad = radians(_latitude);

    // 1. Calculate Solar Declination (the "North-South bias")
    double declinationAngleRad = radians(-23.45) * cos(radians(360.0 / 365.0 * (dayOfYear + 10)));

    // 2. Calculate Equation of Time (in minutes)
    double B = radians(360.0 / 365.0 * (dayOfYear - 81));
    double eot = 9.87 * sin(2 * B) - 7.53 * cos(B) - 1.5 * sin(B);

    // 3. Calculate Time Correction Factor (in minutes)
    float lstm = 15.0 * _timezone; // Local Standard Time Meridian
    double tcf = 4.0 * (_longitude - lstm) + eot;

    // 4. Calculate Local Solar Time (LST)
    double lst = fractionalHour + tcf / 60.0;

    // 5. Calculate Hour Angle (HRA)
    double hraRad = radians(15.0 * (lst - 12.0));

    // 6. Calculate Elevation Angle
    double elevationRad = asin(sin(declinationAngleRad) * sin(latRad) +
                               cos(declinationAngleRad) * cos(latRad) * cos(hraRad));
    _elevation = degrees(elevationRad);

    // 7. Calculate Azimuth Angle
    double azimuthRad = acos((sin(declinationAngleRad) * cos(latRad) -
                              cos(declinationAngleRad) * sin(latRad) * cos(hraRad)) /
                             cos(elevationRad));
    _azimuth = degrees(azimuthRad);

    if (sin(hraRad) > 0)
    {
        _azimuth = 360.0 - _azimuth;
    }
}

#define TEST_CASE
#ifndef TEST_CASE
SunTracker sunTracker;

// Variables to hold the simulated time
timeObject simulatedTime;

// Timer variables for mocking time
unsigned long previousMillis = 0;
const long interval = 1000; // 1 second real-time interval

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ; // Wait for Serial Monitor to open

    Serial.println("\n--- SunTracker Class Test ---");
    Serial.println("Simulating a full day. Time will advance by 30 minutes every second.");

    // Set the starting date for the simulation (using today's date)
    simulatedTime.day = 21;
    simulatedTime.month = 8;
    simulatedTime.year = 25; // For 2025
    simulatedTime.hour = 0;
    simulatedTime.minute = 0;
    simulatedTime.second = 0;

    Serial.println("\nSimulated Time\t\tAzimuth (deg)\tElevation (deg)");
    Serial.println("==========================================================");
}

void loop()
{
    // Check if one second of real time has passed
    if (millis() - previousMillis >= interval)
    {
        previousMillis = millis(); // Reset the timer

        // 1. Update the SunTracker with the current simulated time
        sunTracker.update(simulatedTime);

        // 2. Get the results
        float azimuth = sunTracker.getAzimuth();
        float elevation = sunTracker.getElevation();

        // 3. Print the formatted output
        char timeBuffer[20];
        sprintf(timeBuffer, "%02d/%02d/%02d %02d:%02d:%02d",
                simulatedTime.day, simulatedTime.month, simulatedTime.year,
                simulatedTime.hour, simulatedTime.minute, simulatedTime.second);

        Serial.print(timeBuffer);
        Serial.print("\t");
        Serial.print(azimuth, 2); // Print with 2 decimal places
        Serial.print("\t\t");
        Serial.print(elevation, 2);

        // Print a marker for when the sun is up
        if (elevation > 0)
        {
            Serial.println("\t<-- Sun is up");
        }
        else
        {
            Serial.println();
        }

        // 4. Advance the simulated time by 30 minutes
        simulatedTime.minute += 30;
        if (simulatedTime.minute >= 60)
        {
            simulatedTime.minute = 0;
            simulatedTime.hour++;
            if (simulatedTime.hour >= 24)
            {
                simulatedTime.hour = 0;
                Serial.println("--- 24 hours simulated, resetting day. ---");
            }
        }
    }
}
#endif
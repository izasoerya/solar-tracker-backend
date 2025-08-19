#include <Arduino.h>
#include <sensor_rtc.h>
#include <math.h> // For trigonometric functions

// =================================================================
// SunTracker Class Definition
// =================================================================
class SunTracker
{
public:
    // Constructor: Initializes the tracker with a specific location
    SunTracker();

    // Main calculation method: Updates the sun's position based on the given time
    void update(const timeObject &time);

    // Getter methods to retrieve the calculated angles
    float getAzimuth() const;
    float getElevation() const;

private:
    // --- Configuration (set once at creation) ---
    const float _latitude;
    const float _longitude;
    const float _timezone;

    // --- State Variables (updated by calling update()) ---
    float _azimuth;
    float _elevation;

    // --- Private Helper Methods ---
    int calculateDayOfYear(byte day, byte month /*, int year */);
    void calculateSunAngles(int dayOfYear, float fractionalHour);
};

// =================================================================
// SunTracker Class Implementation
// =================================================================

/**
 * @brief Constructor for the SunTracker class.
 * @param latitude The latitude of the location in decimal degrees.
 * @param longitude The longitude of the location in decimal degrees.
 * @param timezone The UTC timezone offset of the location.
 */
SunTracker::SunTracker()
    : _latitude(-7),
      _longitude(1),
      _timezone(7),
      _azimuth(0.0),
      _elevation(0.0)
{
}

/**
 * @brief Updates the internal azimuth and elevation angles for the given time.
 * @param time A timeObject struct containing the date and time.
 */
void SunTracker::update(const timeObject &time)
{
    // 1. Calculate day of the year.
    // NOTE: This helper function does not account for leap years without a 'year' parameter.
    int dayOfYear = calculateDayOfYear(time.day, time.month);

    // 2. Convert time to a fractional hour for calculations.
    // NOTE: Precision is lost here without minutes and seconds.
    float fractionalHour = static_cast<float>(time.hour);

    // 3. Perform the main astronomical calculations.
    calculateSunAngles(dayOfYear, fractionalHour);
}

/**
 * @brief Gets the last calculated Azimuth angle.
 * @return Azimuth in degrees (0=N, 90=E, 180=S, 270=W).
 */
float SunTracker::getAzimuth() const
{
    return _azimuth;
}

/**
 * @brief Gets the last calculated Elevation angle.
 * @return Elevation in degrees (angle above the horizon).
 */
float SunTracker::getElevation() const
{
    return _elevation;
}

// --- Private Methods Implementation ---

/**
 * @brief Calculates the day of the year (1-365).
 * WARNING: This implementation does not account for leap years!
 */
int SunTracker::calculateDayOfYear(byte day, byte month /*, int year */)
{
    const int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int doy = 0;
    // To add leap year support:
    // if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
    //     daysInMonth[2] = 29;
    // }
    for (int i = 1; i < month; ++i)
    {
        doy += daysInMonth[i];
    }
    doy += day;
    return doy;
}

/**
 * @brief The core solar position calculation logic. Updates private member variables.
 * @param dayOfYear The current day of the year (1-366).
 * @param fractionalHour The current hour of the day (e.g., 15.5 for 3:30 PM).
 */
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

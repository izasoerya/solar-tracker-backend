#include <Wire.h>
#include <avr/wdt.h>

#include "filter.h"
#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "sensor_rtc.h"
#include "control_system.h"
#include "sun_trajectory.h"
#include "rtc_makeshift.h"

#define STEP 1
#define VAL_MIN -60
#define VAL_MAX 60

UserInterface ui;
UserInput input;
SensorFXOSFXAS mpu;
byte ldrPins[6] = {A0, A1, A2, A3, A6, A7};
SensorLDR ldr(ldrPins);
ControlSystem control;
LowPassFilter lp[6];
SunTracker sun;
SensorRTC rtc;
timeObject nows;
// RTCMakeshift mockRTC;

// === Timing trackers ===
unsigned long lastUI = 0;
unsigned long lastSensors = 0;
unsigned long lastControl = 0;
unsigned long lastInput = 0;

// === Intervals ===
const unsigned long UI_INTERVAL = 1000;	   // 1s
const unsigned long SENS_INTERVAL = 100;   // 100ms
const unsigned long CONTROL_INTERVAL = 20; // 20ms
const unsigned long INPUT_INTERVAL = 5;	   // 5ms

AppState appState = AppState::AUTOMATIC;
ManualSelection selection = ManualSelection::X;
bool inEditMode = false;
bool inLDRMode = false;

unsigned long bootMillis = 0;
bool allowWDT = true;

int8_t xVal = 0;
int8_t yVal = 0;
float angleMain = 0;
float angleSecond = 0;
byte sunWest = 0;
byte sunSouth = 0;
byte sunEast = 0;
byte sunNorth = 0;

// === Function Prototypes ===
void handleUI();
void handleSensorUpdate();
void handleControl();
void handleInput();

// === UI Update Task ===
void handleUI()
{
	if (appState == AppState::AUTOMATIC)
	{
		ui.showDebugLDR(sunWest, sunSouth,
						sunEast, sunNorth,
						nows, inLDRMode);
	}
	else
	{
		ui.showManual(
			sunWest, sunSouth, sunEast, sunNorth,
			xVal, yVal,
			(angleMain), (angleSecond),
			selection, inEditMode);
	}
}

// === Sensor Update Task ===
void handleSensorUpdate()
{
	mpu.update();
	ldr.update();
	rtc.update();
	// mockRTC.update();
	nows = rtc.getData();
	sun.update(nows);

	sunWest = lp[0].reading(ldr.getRawValue(0));
	sunEast = lp[1].reading(ldr.getRawValue(1));
	sunSouth = lp[2].reading(ldr.getRawValue(2));
	sunNorth = lp[3].reading(ldr.getRawValue(3));
	angleMain = lp[4].reading(mpu.getAccelRoll() * 1.028 - 0.113);
	angleSecond = lp[5].reading(mpu.getAccelPitch() + 0.2);
}

// === Control Actuator Task ===
void handleControl()
{
	wdt_reset();
	if (appState == AppState::AUTOMATIC)
	{
		// -- Cloudy sky -- //
		// static byte refSunLeft = 0;
		// static byte refSunRight = 0;
		// bool isCloudy = (sunWest < 50 && sunSouth < 50 && sunEast < 50 && sunNorth < 50);
		// if (isCloudy)
		// {
		// 	control.cloudyStrategy(
		// 		millis(), ldr.getRawValue(4), ldr.getRawValue(5),
		// 		refSunLeft, refSunRight, sunEast, sunNorth);
		// }
		// else
		// {
		// 	refSunLeft = sunEast;
		// 	refSunRight = sunNorth;
		// }
		if ((nows.hour >= 20 && nows.hour <= 23) || (nows.hour >= 0 && nows.hour <= 3))
		{
			control.runManual(0, 0, angleMain, angleSecond);
		}
		else
		{
			float targetElevation = sun.getElevation();
			float targetAzimuth = sun.getAzimuth();

			if (sun.getElevation() < 0 && nows.hour > 3)
			{
				// Serial.println("Outside Time");
				control.runManual(-60, 0, angleMain, angleSecond);
				return;
			}

			else
			{
				SeptyanJaya angle = sun.septyanUpdate(targetAzimuth, targetElevation);
				bool xInThreshold = fabs(angle.parsedX - angleMain) <= 10;
				bool yInThreshold = fabs(angle.parsedY - angleSecond) <= 10;
				if (!xInThreshold)
				{
					control.runX(angle.parsedX, angleMain);
				}
				if (!yInThreshold)
				{
					control.runY(angle.parsedY, angleSecond);
				}
				// Serial.print("X: ");
				// Serial.print(angleMain);
				// Serial.print("  Y: ");
				// Serial.print(angleSecond);
				// Serial.print("  TX: ");
				// Serial.print(angle.parsedX);
				// Serial.print("  TY: ");
				// Serial.print(angle.parsedY);
				// Serial.print("       ");
				// Serial.print(rtc.getData().hour);
				// Serial.print(":");
				// Serial.print(rtc.getData().minute);
				// Serial.print(":");
				// Serial.println(rtc.getData().second);

				if (xInThreshold && yInThreshold)
				{
					// Serial.println("LDR Adjustment");
					inLDRMode = true;
					float diffMain = sunWest - sunEast;
					float diffSecond = sunSouth - sunNorth;

					float deadbandMain = (min(sunWest, sunEast) / max(sunWest, sunEast));
					float deadbandSecond = (min(sunSouth, sunNorth) / max(sunSouth, sunNorth));

					float angleParsedXOverflow = angle.parsedX;
					float angleParsedYOverflow = angle.parsedY;

					if (deadbandMain < 0.9)
					{
						angleParsedXOverflow += (diffMain > 0) ? 0.25 : -0.25;
					}
					if (deadbandSecond < 0.9)
					{
						angleParsedYOverflow += (diffSecond > 0) ? 0.25 : -0.25;
					}
					control.runManual(angleParsedXOverflow, angleParsedYOverflow, (angleMain + 0.113) / 1.028, angleSecond - 0.2);
				}
				else
				{
					inLDRMode = false;
				}
			}
		}
	}

	else if (appState == AppState::MANUAL)
	{
		control.runManual(xVal, yVal, angleMain, angleSecond);
	}
}

// === Input Handling Task ===
void handleInput()
{
	input.update();

	if (appState == AppState::AUTOMATIC)
	{
		if (input.wasPressed())
		{
			appState = AppState::MANUAL;
			selection = ManualSelection::X;
			inEditMode = false;
		}
	}
	else if (appState == AppState::MANUAL)
	{
		if (input.wasPressed())
		{
			if (selection == ManualSelection::BACK)
			{
				appState = AppState::AUTOMATIC;
				inEditMode = false;
			}
			else
			{
				inEditMode = !inEditMode;
			}
		}

		int8_t dir = input.getDirection();
		if (dir != 0)
		{
			if (inEditMode)
			{
				if (selection == ManualSelection::X)
					xVal = constrain(xVal + dir * STEP, VAL_MIN, VAL_MAX);
				else if (selection == ManualSelection::Y)
					yVal = constrain(yVal + dir * STEP, VAL_MIN, VAL_MAX);
			}
			else
			{
				int newSel = static_cast<int>(selection) + dir;
				newSel = constrain(newSel, 0, static_cast<int>(ManualSelection::COUNT) - 1);
				selection = static_cast<ManualSelection>(newSel);
			}
		}
	}
}

void setup()
{
	// Serial.begin(115200);
	Wire.begin();
	Wire.setClock(10000);

	ui.init();
	input.init();
	mpu.begin();
	ldr.begin();
	rtc.begin();
	// mockRTC.begin();

	wdt_disable();
	delay(2000);
	wdt_enable(WDTO_120MS);
	bootMillis = millis();
	allowWDT = true;
}

void loop()
{
	unsigned long now = millis();
	if (allowWDT && (now - bootMillis >= 900000UL))
	{
		allowWDT = false;
	}

	if (now - lastUI >= UI_INTERVAL)
	{
		lastUI = now;
		handleUI();
	}

	if (now - lastSensors >= SENS_INTERVAL)
	{
		lastSensors = now;
		handleSensorUpdate();
	}

	if (now - lastControl >= CONTROL_INTERVAL)
	{
		lastControl = now;
		if (allowWDT)
		{
			wdt_reset();
		}
		handleControl();
	}

	if (now - lastInput >= INPUT_INTERVAL)
	{
		lastInput = now;
		handleInput();
	}
}
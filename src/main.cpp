#include <Wire.h>
#include <TaskScheduler.h>
#include <avr/wdt.h>

#include "filter.h"
#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "sensor_rtc.h"
#include "control_system.h"
#include "sun_trajectory.h"
// #include "rtc_makeshift.h"

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

AppState appState = AppState::AUTOMATIC;
ManualSelection selection = ManualSelection::X;
bool inEditMode = false;
bool inLDRMode = false;

int8_t xVal = 0;
int8_t yVal = 0;
float angleMain = 0;
float angleSecond = 0;
byte sunWest = 0;
byte sunSouth = 0;
byte sunEast = 0;
byte sunNorth = 0;

Scheduler scheduler;

// === Function Prototypes ===
void handleUI();
void handleSensorUpdate();
void handleControl();
void handleInput();

// === Tasks ===
Task serveUI(500, TASK_FOREVER, &handleUI);				  // UI updates every 1 second
Task updateSensors(8, TASK_FOREVER, &handleSensorUpdate); // Sensor updates every 10ms
Task controlTask(20, TASK_FOREVER, &handleControl);		  // Control every 20ms
Task inputTask(5, TASK_FOREVER, &handleInput);			  // Input reading every 5ms

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
	angleMain = lp[4].reading(mpu.getAccelRoll());
	angleSecond = lp[5].reading(mpu.getAccelPitch());
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

			if (sun.getElevation() < 0)
			{
				Serial.println("Outside Time");
				return;
			}

			else
			{
				SeptyanJaya angle = sun.septyanUpdate(targetAzimuth, targetElevation);
				bool xInThreshold = fabs(angle.parsedX - angleMain) <= 15;
				bool yInThreshold = fabs(angle.parsedY - angleSecond) <= 10;
				if (!xInThreshold)
				{
					control.runX(angle.parsedX, angleMain);
				}
				if (!yInThreshold)
				{
					control.runY(angle.parsedY, angleSecond);
				}
				Serial.print("X: ");
				Serial.print(angleMain);
				Serial.print("  Y: ");
				Serial.print(angleSecond);
				Serial.print("  TX: ");
				Serial.print(angle.parsedX);
				Serial.print("  TY: ");
				Serial.print(angle.parsedY);
				Serial.print("       ");
				Serial.print(rtc.getData().hour);
				Serial.print(":");
				Serial.print(rtc.getData().minute);
				Serial.print(":");
				Serial.println(rtc.getData().second);

				if (xInThreshold && yInThreshold)
				{
					Serial.println("LDR Adjustment");
					inLDRMode = true;
					float diffMain = sunWest - sunEast;
					float diffSecond = sunSouth - sunNorth;

					const float LDR_DEADBAND_PERCENT = 0.05f; // 5%
					float deadbandMain = LDR_DEADBAND_PERCENT * max(sunWest, sunEast);
					float deadbandSecond = LDR_DEADBAND_PERCENT * max(sunSouth, sunNorth);

					float angleParsedXOverflow = angle.parsedX;
					float angleParsedYOverflow = angle.parsedY;

					if (fabs(diffMain) > deadbandMain)
					{
						angleParsedXOverflow += (diffMain > 0) ? 1 : -1;
					}
					if (fabs(diffSecond) > deadbandSecond)
					{
						angleParsedYOverflow += (diffSecond > 0) ? 1 : -1;
					}
					control.runManual(angleParsedXOverflow, angleParsedYOverflow, angleMain, angleSecond);
				}
				else
				{
					inLDRMode = false;
				}
			}
		}

		// // -- Normal sky -- //
		// float diffMain = sunWest - sunEast;
		// float diffSecond = sunSouth - sunNorth;
		// control.runAutomatic(diffMain, diffSecond);
	}

	else if (appState == AppState::MANUAL)
	{
		float raw_target_roll = xVal;
		float raw_target_pitch = yVal;
		control.runManual(raw_target_roll, raw_target_pitch, angleMain, angleSecond);
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

		int dir = input.getDirection();
		if (dir != 0)
		{
			if (inEditMode)
			{
				if (selection == ManualSelection::X)
					xVal = constrain(xVal + dir * STEP * -1, VAL_MIN, VAL_MAX);
				else if (selection == ManualSelection::Y)
					yVal = constrain(yVal + dir * STEP * -1, VAL_MIN, VAL_MAX);
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
	Serial.begin(115200);
	Wire.begin();

	ui.init();
	input.init();
	mpu.begin();
	mpu.setGyroSensitivity(0);
	mpu.setAccelSensitivity(0);
	mpu.setFilterBandwidth(4);
	ldr.begin();
	rtc.begin();
	// mockRTC.begin();
	wdt_disable();
	delay(2L * 1000L);
	wdt_enable(WDTO_120MS);

	// Test Septyan Angle Calculation
	// auto s0 = sun.septyanUpdate(77.51, 0.05);
	// auto s1 = sun.septyanUpdate(73.6, 20.42);
	// auto s2 = sun.septyanUpdate(69.1, 34.5);
	// auto s3 = sun.septyanUpdate(61.0, 48.0);
	// auto s4 = sun.septyanUpdate(300.0, 50.0);
	// auto s5 = sun.septyanUpdate(288.0, 56.0);
	// auto DOWN = sun.septyanUpdate(200, 0.13);
	// auto TOP = sun.septyanUpdate(0.11, 70.0);
	// Serial.print("TOPX: ");
	// Serial.print(TOP.parsedX);
	// Serial.print(" TOPY: ");
	// Serial.print(TOP.parsedY);
	// Serial.print(" DOWNX: ");
	// Serial.print(DOWN.parsedX);
	// Serial.print(" DOWNY: ");
	// Serial.print(DOWN.parsedY);
	// Serial.print("     ");
	// Serial.print("X0: ");
	// Serial.print(s0.parsedX);
	// Serial.print(" Y0: ");
	// Serial.print(s0.parsedY);
	// Serial.print("     ");
	// Serial.print("X0: ");
	// Serial.print(s0.parsedX);
	// Serial.print(" Y0: ");
	// Serial.print(s0.parsedY);
	// Serial.print("     ");
	// Serial.print("X4: ");
	// Serial.print(s4.parsedX);
	// Serial.print(" Y4: ");
	// Serial.print(s4.parsedY);
	// Serial.print("     ");
	// Serial.print("X5: ");
	// Serial.print(s5.parsedX);
	// Serial.print(" Y5: ");
	// Serial.println(s5.parsedY);

	scheduler.init();
	scheduler.addTask(serveUI);
	scheduler.addTask(updateSensors);
	scheduler.addTask(controlTask);
	scheduler.addTask(inputTask);

	serveUI.enable();
	updateSensors.enable();
	controlTask.enable();
	inputTask.enable();
}

void loop()
{
	scheduler.execute();
}
#include <Wire.h>
#include <TaskScheduler.h>

#include "filter.h"
#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "sensor_rtc.h"
#include "madgwick_imu.h"
#include "control_system.h"
#include "sun_trajectory.h"

#define STEP 1
#define VAL_MIN -60
#define VAL_MAX 60

UserInterface ui;
UserInput input;
SensorFXOSFXAS mpu;
byte ldrPins[6] = {A0, A1, A2, A3, A6, A7};
SensorLDR ldr(ldrPins);
ControlSystem control;
MadgwickIMU madgwick;
LowPassFilter lp[4];
SunTracker sun;
SensorRTC rtc;

AppState appState = AppState::AUTOMATIC;
ManualSelection selection = ManualSelection::X;
bool inEditMode = false;

int8_t xVal = 0;
int8_t yVal = 0;
float angleX = 0;
float angleY = 0;
byte sunTop = 0;
byte sunBot = 0;
byte sunLeft = 0;
byte sunRight = 0;

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
		// ui.showAutomatic(
		// 	(sunX + sunY) / 2,
		// 	angleX, angleY);

		ui.showDebugLDR(sunTop,
						sunBot,
						sunLeft,
						sunRight);
	}
	else
	{
		ui.showManual(
			sunTop, sunBot, sunLeft, sunRight,
			xVal, yVal,
			(angleX * 1.268 + 0.547) * 1.06 - 1.12, (angleY * 1.326 + 0.233) * 0.98 + 0.27,
			selection, inEditMode);
	}
}

// === Sensor Update Task ===
void handleSensorUpdate()
{
	if (appState == AppState::MANUAL)
	{
		mpu.update();
		ldr.update();

		ModelIMU imuData = mpu.getModelIMU();
		madgwick.update(imuData);

		angleX = madgwick.getRoll();
		angleY = madgwick.getPitch();
	}
	else
	{
		ldr.update();
		sunTop = lp[0].reading(ldr.getRawValue(0));
		sunLeft = lp[1].reading(ldr.getRawValue(1));
		sunBot = lp[2].reading(ldr.getRawValue(2));
		sunRight = lp[3].reading(ldr.getRawValue(3));
	}
}

// === Control Actuator Task ===
void handleControl()
{
	static unsigned long cloudyStartTime = 0;
	static bool inCloudyFallback = false;
	static byte seeker1Stable = 0;
	static byte seeker2Stable = 0;

	if (appState == AppState::AUTOMATIC)
	{
		// -- Cloudy sky -- //
		bool isCloudy = (sunTop < 50 && sunBot < 50 && sunLeft < 50 && sunRight < 50);
		if (isCloudy)
		{
			if (cloudyStartTime == 0)
			{
				cloudyStartTime = millis();
				seeker1Stable = 0;
				seeker2Stable = 0;
			}
			if ((millis() - cloudyStartTime) >= 30000)
				inCloudyFallback = true;
			return; // Do not run normal control while in fallback
		}
		else
		{
			cloudyStartTime = 0;
			inCloudyFallback = false;
			seeker1Stable = 0;
			seeker2Stable = 0;
		}

		if (inCloudyFallback)
		{
			byte seeker1 = ldr.getRawValue(5);
			byte seeker2 = ldr.getRawValue(6);

			if (seeker1 > 120 && seeker2 > 120)
			{
				seeker1Stable = seeker1;
				seeker2Stable = seeker2;
				bool approved = control.runFallBackStrategy(seeker1Stable, seeker2Stable, sunTop, sunBot);
				if (approved)
				{
					inCloudyFallback = false;
					cloudyStartTime = 0;
					seeker1Stable = 0;
					seeker2Stable = 0;
				}
			}
			return; // Do not run normal control while in fallback
		}

		// -- Normal sky -- //
		float diffX = sunTop - sunLeft;
		float diffY = sunBot - sunRight;
		control.runAutomatic(diffX, diffY);
	}
	else if (appState == AppState::MANUAL)
	{
		float raw_target_roll = (((xVal - 0.547) / 1.268) + 1.12) / 1.06;
		float raw_target_pitch = (((yVal - 0.233) / 1.326) - 0.27) / 0.98;
		control.runManual(raw_target_roll, raw_target_pitch, madgwick.getRoll(), madgwick.getPitch());
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
	Wire.begin();
	// Wire.setTimeout(100000);

	ui.init();
	input.init();
	mpu.begin();
	mpu.setGyroSensitivity(0);
	mpu.setAccelSensitivity(0);
	mpu.setFilterBandwidth(4);
	ldr.begin();

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
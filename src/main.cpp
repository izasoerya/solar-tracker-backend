#include <Wire.h>
#include <TaskScheduler.h>

#include "filter.h"
#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "sensor_rtc.h"
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
LowPassFilter lp[6];
SunTracker sun;
SensorRTC rtc;
timeObject nows;

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

		ui.showDebugLDR(sunTop, sunBot,
						sunLeft, sunRight,
						nows);
	}
	else
	{
		ui.showManual(
			sunTop, sunBot, sunLeft, sunRight,
			xVal, yVal,
			(angleX), (angleY),
			selection, inEditMode);
	}
}

// === Sensor Update Task ===
void handleSensorUpdate()
{
	rtc.update();
	nows = rtc.getData();
	if (appState == AppState::MANUAL)
	{
		mpu.update();
		ldr.update();

		angleX = lp[4].reading(mpu.getAccelRoll());
		angleY = lp[5].reading(mpu.getAccelPitch());
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

	if (appState == AppState::AUTOMATIC)
	{
		// -- Cloudy sky -- //
		// static byte refSunLeft = 0;
		// static byte refSunRight = 0;
		// bool isCloudy = (sunTop < 50 && sunBot < 50 && sunLeft < 50 && sunRight < 50);
		// if (isCloudy)
		// {
		// 	control.cloudyStrategy(
		// 		millis(), ldr.getRawValue(4), ldr.getRawValue(5),
		// 		refSunLeft, refSunRight, sunLeft, sunRight);
		// }
		// else
		// {
		// 	refSunLeft = sunLeft;
		// 	refSunRight = sunRight;
		// }

		// -- Normal sky -- //
		float diffX = sunTop - sunLeft;
		float diffY = sunBot - sunRight;
		control.runAutomatic(diffX, diffY);
	}

	else if (appState == AppState::MANUAL)
	{
		float raw_target_roll = (((xVal)));
		float raw_target_pitch = (((yVal)));
		control.runManual(raw_target_roll, raw_target_pitch, angleX, angleY);
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
	rtc.begin();

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
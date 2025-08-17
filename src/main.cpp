#include <Wire.h>
#include <TaskScheduler.h>
#include <avr/wdt.h>

#include "filter.h"
#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "madgwick_imu.h"
#include "control_system.h"
#include "state_save.h"

#define STEP 1
#define VAL_MIN -60
#define VAL_MAX 60

UserInterface ui;
UserInput input;
SensorMPU mpu;
byte ldrPins[6] = {A0, A1, A2, A3, A6, A7};
SensorLDR ldr(ldrPins);
ControlSystem control;
MadgwickIMU imu;
LowPassFilter lp[4];
SystemStructure deviceSystem;
StateSave stateSystem;
Scheduler scheduler;

float angleX = 0;
float angleY = 0;
byte sunTop = 0;
byte sunBot = 0;
byte sunLeft = 0;
byte sunRight = 0;

// === Function Prototypes ===
void handleUI();
void handleSensorUpdate();
void handleControl();
void handleInput();

// === Tasks ===
Task serveUI(1000, TASK_FOREVER, &handleUI);			  // UI updates every 1 second
Task updateSensors(8, TASK_FOREVER, &handleSensorUpdate); // Sensor updates every 10ms
Task controlTask(20, TASK_FOREVER, &handleControl);		  // Control every 20ms
Task inputTask(5, TASK_FOREVER, &handleInput);			  // Input reading every 5ms

// === UI Update Task ===
void handleUI()
{
	if (deviceSystem.state == AppState::AUTOMATIC)
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
			deviceSystem.xVal, deviceSystem.yVal,
			angleX * 1.268 + 0.547, angleY * 1.326 + 0.233,
			deviceSystem.selection, deviceSystem.inEditMode);
	}
}

// === Sensor Update Task ===
void handleSensorUpdate()
{
	mpu.update();
	ldr.update();

	ModelIMU imuData = mpu.getModelIMU();
	imu.update(imuData);

	angleX = imu.getRoll();
	angleY = imu.getPitch();
	sunTop = lp[0].reading(ldr.getRawValue(0) * 0.86);
	sunLeft = lp[1].reading(ldr.getRawValue(1) * 0.765);
	sunBot = lp[2].reading(ldr.getRawValue(2));
	sunRight = lp[3].reading(ldr.getRawValue(3));
	wdt_reset();
}

void handleControl()
{
	if (deviceSystem.state == AppState::AUTOMATIC)
	{
		float diffX = sunTop - sunBot;
		float diffY = sunLeft - sunRight;
		control.runAutomatic(diffX, diffY);
	}
	else if (deviceSystem.state == AppState::MANUAL)
	{
		float raw_target_roll = (deviceSystem.xVal - 0.547) / 1.268;
		float raw_target_pitch = (deviceSystem.yVal - 0.233) / 1.326;
		control.runManual(raw_target_roll, raw_target_pitch, imu.getRoll(), imu.getPitch());
	}
}

// === Input Handling Task ===
void handleInput()
{
	input.update();

	if (deviceSystem.state == AppState::AUTOMATIC)
	{
		if (input.wasPressed())
		{
			deviceSystem.state = AppState::MANUAL;
			deviceSystem.selection = ManualSelection::X;
			deviceSystem.inEditMode = false;
		}
	}
	else if (deviceSystem.state == AppState::MANUAL)
	{
		if (input.wasPressed())
		{
			if (deviceSystem.selection == ManualSelection::BACK)
			{
				deviceSystem.state = AppState::AUTOMATIC;
				deviceSystem.inEditMode = false;
			}
			else
			{
				deviceSystem.inEditMode = !deviceSystem.inEditMode;
			}
		}

		int dir = input.getDirection();
		if (dir != 0)
		{
			if (deviceSystem.inEditMode)
			{
				if (deviceSystem.selection == ManualSelection::X)
					deviceSystem.xVal = constrain(deviceSystem.xVal + dir * STEP * -1, VAL_MIN, VAL_MAX);
				else if (deviceSystem.selection == ManualSelection::Y)
					deviceSystem.yVal = constrain(deviceSystem.yVal + dir * STEP * -1, VAL_MIN, VAL_MAX);
			}
			else
			{
				int newSel = static_cast<int>(deviceSystem.selection) + dir;
				newSel = constrain(newSel, 0, static_cast<int>(ManualSelection::COUNT) - 1);
				deviceSystem.selection = static_cast<ManualSelection>(newSel);
			}
		}
	}
}

ISR(WDT_vect)
{
	stateSystem.updateState(deviceSystem);
}

void setup()
{
	Serial.begin(115200);
	Wire.begin();
	Wire.setWireTimeout(25000); // 25ms timeout instead of infinite block

	ui.init();
	input.init();
	mpu.begin();
	mpu.setGyroSensitivity(0);
	mpu.setAccelSensitivity(0);
	mpu.setFilterBandwidth(4);
	ldr.begin();
	stateSystem.initialization();
	deviceSystem = stateSystem.getState();
	wdt_disable(); /* Disable the watchdog and wait for more than 2 seconds */
	delay(3000);   /* Done so that the Arduino doesn't keep resetting infinitely in case of wrong configuration */
	wdt_enable(WDTO_60MS);

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
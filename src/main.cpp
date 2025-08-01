#include <Wire.h>
#include <TaskScheduler.h>

#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "madgwick_imu.h"
#include "control_system.h"

#define STEP 1
#define VAL_MIN -60
#define VAL_MAX 60

UserInterface ui;
UserInput input;
SensorMPU mpu;
byte ldrPins[6] = {A0, A1, A2, A3, A4, A5};
SensorLDR ldr(ldrPins);
ControlSystem control;
MadgwickIMU imu;

AppState appState = AppState::AUTOMATIC;
ManualSelection selection = ManualSelection::X;
bool inEditMode = false;

int8_t xVal = 0;
int8_t yVal = 0;
float angleX = 0;
float angleY = 0;
uint8_t sunX = 0;
uint8_t sunY = 0;

Scheduler scheduler;

// === Function Prototypes ===
void handleUI();
void handleSensorUpdate();
void handleControl();
void handleInput();

// === Tasks ===
Task serveUI(1000, TASK_FOREVER, &handleUI);			   // UI updates every 1 second
Task updateSensors(10, TASK_FOREVER, &handleSensorUpdate); // Sensor updates every 10ms
Task controlTask(20, TASK_FOREVER, &handleControl);		   // Control every 20ms
Task inputTask(5, TASK_FOREVER, &handleInput);			   // Input reading every 5ms

// === UI Update Task ===
void handleUI()
{
	if (appState == AppState::AUTOMATIC)
	{
		ui.showAutomatic(
			(sunX + sunY) / 2,
			angleX, angleY);
	}
	else
	{
		ui.showManual(
			(sunX + sunY) / 2,
			xVal, yVal,
			angleX, angleY,
			selection, inEditMode);
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
	sunX = ldr.getSumX();
	sunY = ldr.getSumY();
}

// === Control Task ===
void handleControl()
{
	if (appState == AppState::AUTOMATIC)
	{
		// control.runAutomatic(sunX, sunY);
	}
	else if (appState == AppState::MANUAL)
	{
		control.runManual(xVal, yVal, angleX, angleY);
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
	mpu.setGyroSensitivity(1);
	mpu.setAccelSensitivity(2);
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

// #include <Arduino.h>
// #include <motor.h>

// Motor driverX(3, 4, 5, 6);
// Motor driverY(7, 8, 9, 10);

// void setup()
// {
// }

// void loop()
// {
// 	driverY.turnRight(255);
// 	delay(10000);
// 	driverY.turnLeft(255);
// 	delay(10000);
// }
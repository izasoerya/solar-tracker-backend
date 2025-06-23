#include <Wire.h>
#include <TaskScheduler.h>

#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"
#include "control_system.h"

#define STEP 1
#define VAL_MIN -90
#define VAL_MAX 90

UserInterface ui;
UserInput input;
SensorMPU mpu;
byte ldrPins[6] = {A0, A1, A2, A3, A4, A5};
SensorLDR ldr(ldrPins);
ControlSystem control;

AppState appState = AppState::AUTOMATIC;
ManualSelection selection = ManualSelection::X;
bool inEditMode = false;

int8_t xVal = 0;
int8_t yVal = 0;
float angleX = 0;
float angleY = 0;
uint16_t sunX = 0;
uint16_t sunY = 0;

Scheduler scheduler;

Task serveUI(500, TASK_FOREVER, []()
			 {
    if (appState == AppState::AUTOMATIC) {
        ui.showAutomatic(
			(sunX + sunY) / 2,
			angleX, angleY
		);
    } else {
        ui.showManual(
			(sunX + sunY) / 2,
			xVal, yVal,
			angleX, angleY,
			selection, inEditMode
		);
    } });

void setup()
{
	Serial.begin(115200);
	Wire.begin();

	ui.init();
	input.init();
	mpu.begin();
	ldr.begin();

	scheduler.init();
	scheduler.addTask(serveUI);
	serveUI.enable();
}

void loop()
{
	scheduler.execute();
	input.update();
	mpu.update();
	ldr.update();

	angleX = mpu.getRoll();
	angleY = mpu.getPitch();
	sunX = ldr.getSumX();
	sunY = ldr.getSumY();

	if (appState == AppState::AUTOMATIC)
	{
		if (input.wasPressed())
		{
			appState = AppState::MANUAL;
			selection = ManualSelection::X;
			inEditMode = false;
		}
		control.runAutomatic(sunX, sunY);
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
				if (newSel < 0)
					newSel = 0;
				if (newSel >= static_cast<int>(ManualSelection::COUNT))
					newSel = static_cast<int>(ManualSelection::COUNT) - 1;
				selection = static_cast<ManualSelection>(newSel);
			}
		}
		control.runManual(xVal, yVal, mpu.getRoll(), mpu.getPitch());
		delay(5);
	}
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
// 	driverY.turnRight(155);
// }
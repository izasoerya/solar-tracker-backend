#include <Wire.h>
#include <TaskScheduler.h>

#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"
#include "sensor_ldr.h"

#define STEP 5
#define VAL_MIN 0
#define VAL_MAX 100

UserInterface ui;
UserInput input;
SensorMPU mpu;
byte ldrPins[6] = {A0, A1, A2, A3, A4, A5};
SensorLDR ldr(ldrPins);

AppState appState = AppState::AUTOMATIC;
ManualSelection selection = ManualSelection::X;
bool inEditMode = false;

int xVal = 0;
int yVal = 0;
uint16_t sunX = 0;
uint16_t sunY = 0;

Scheduler scheduler;

Task taskUI(500, TASK_FOREVER, []()
			{
    if (appState == AppState::AUTOMATIC) {
        ui.showAutomatic((sunX + sunY) / 2, xVal, yVal);
    } else {
        ui.showManual((sunX + sunY) / 2, xVal, yVal, selection, inEditMode);
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
	scheduler.addTask(taskUI);
	taskUI.enable();
}

void loop()
{
	scheduler.execute();
	input.update();
	mpu.update();
	ldr.update();

	if (appState == AppState::AUTOMATIC)
	{
		xVal = constrain(static_cast<int>(mpu.getRoll()), VAL_MIN, VAL_MAX);
		yVal = constrain(static_cast<int>(mpu.getPitch()), VAL_MIN, VAL_MAX);

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
					xVal = constrain(xVal + dir * STEP, VAL_MIN, VAL_MAX);
				else if (selection == ManualSelection::Y)
					yVal = constrain(yVal + dir * STEP, VAL_MIN, VAL_MAX);
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
	}
}

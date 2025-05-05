#include "user_interface.h"
#include "user_input.h"
#include "sensor_mpu.h"

UserInterface ui;
UserInput input;
SensorMPU mpu; // Assuming you have a class for the MPU

void setup()
{
	Serial.begin(115200);
	ui.init();
	ui.initDisplay();
	mpu.begin();
}

void loop()
{
	mpu.update();

	if (ui.currentState == UserInterfaceState::IDLE)
	{
		ui.idleDisplay(90, mpu.getPitch(), mpu.getRoll());
		ui.moveCursor(true);
		ui.idleDisplay(90, mpu.getPitch(), mpu.getRoll());
		delay(3000);

		ui.moveCursor(false);
		ui.idleDisplay(90, mpu.getPitch(), mpu.getRoll());
		delay(3000);

		ui.moveCursor(true);
		ui.idleDisplay(90, mpu.getPitch(), mpu.getRoll());
		ui.changePage();
		delay(3000);
	}
	else if (ui.currentState == UserInterfaceState::MANUAL)
	{
		ui.manualDisplay(90, mpu.getPitch(), mpu.getRoll());
		ui.moveCursor(false);
		ui.manualDisplay(90, mpu.getPitch(), mpu.getRoll());
		delay(3000);
		ui.moveCursor(false);
		ui.manualDisplay(90, mpu.getPitch(), mpu.getRoll());
		delay(3000);
		ui.changePage();
		ui.moveCursor(false);
		delay(3000);
	}
	else
	{
		ui.initDisplay();
		delay(3000);
	}
}
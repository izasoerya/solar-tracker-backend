#pragma once

#include <LCD_I2C.h>
#include "sensor_rtc.h"

enum class AppState
{
    AUTOMATIC,
    MANUAL
};

enum class ManualSelection
{
    X = 0,
    Y = 1,
    BACK = 2,
    COUNT
};

class UserInterface
{
private:
    LCD_I2C lcd;

public:
    UserInterface() : lcd(0x27, 20, 4) {}

    void init()
    {
        lcd.begin();
        lcd.backlight();
        lcd.clear();
    }

    void showAutomatic(uint8_t sun, float x, float y)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Mode: AUTO");
        lcd.setCursor(0, 1);
        lcd.print("Sun:");
        lcd.print(sun);
        lcd.setCursor(0, 2);
        lcd.print("X:");
        lcd.print(x);
        lcd.setCursor(0, 3);
        lcd.print("Y:");
        lcd.print(y);
    }

    void showManual(
        byte sunTop, byte sunBot, byte sunLeft, byte sunRight,
        int x, int y, float mpuX, float mpuY,
        ManualSelection selection, bool inEdit)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Mode: MANUAL");

        for (int i = 0; i < 3; i++)
        {
            lcd.setCursor(0, i + 1);

            bool selected = (i == static_cast<int>(selection));
            if (i == 0)
            {
                lcd.print((selected ? ">" : " ") + String("X:") + String(x) + (selected && inEdit ? "*" : ""));
                lcd.setCursor(8, 1);
                lcd.print("X=" + String(mpuX));
            }
            else if (i == 1)
            {
                lcd.print((selected ? ">" : " ") + String("Y:") + String(y) + (selected && inEdit ? "*" : ""));
                lcd.setCursor(8, 2);
                lcd.print("Y=" + String(mpuY));
            }
            else
            {
                lcd.print((selected ? ">" : " ") + String("Back"));
            }
        }
    }

    void showDebugLDR(byte top, byte down, byte left, byte right, timeObject now, bool inLDRMode)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(now.hour);
        lcd.print(":");
        lcd.print(now.minute);
        lcd.print(":");
        lcd.print(now.second);
        lcd.print("|");
        lcd.print(now.day);
        lcd.print("/");
        lcd.print(now.month);
        lcd.print("/");
        lcd.print(now.year);
        inLDRMode ? lcd.print("*") : lcd.print("");

        lcd.setCursor(0, 2);
        lcd.print("West:");
        lcd.print(top);
        lcd.setCursor(10, 2);
        lcd.print("South:");
        lcd.print(down);

        lcd.setCursor(0, 3);
        lcd.print("East:");
        lcd.print(left);
        lcd.setCursor(10, 3);
        lcd.print("North:");
        lcd.print(right);
    }
};

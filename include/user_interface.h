#pragma once

#include <LCD_I2C.h>

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

    void showManual(uint8_t sun, int x, int y, float mpuX, float mpuY, ManualSelection selection, bool inEdit)
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

        // Print actual MPU values to Serial
        Serial.print("Actual MPU X: ");
        Serial.print(mpuX);
        Serial.print(" | Actual MPU Y: ");
        Serial.println(mpuY);
    }

    void showDebugLDR(int ldr0, int ldr1, int ldr2, int ldr3, int ldr4, int ldr5)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("LDR Debug:");

        lcd.setCursor(0, 1);
        lcd.print("1:");
        lcd.print(ldr0);
        lcd.print(" 2:");
        lcd.print(ldr1);
        lcd.print(" 3:");
        lcd.print(ldr2);

        lcd.setCursor(0, 2);
        lcd.print("4:");
        lcd.print(ldr3);
        lcd.print(" 5:");
        lcd.print(ldr4);
        lcd.print(" 6:");
        lcd.print(ldr5);
    }
};

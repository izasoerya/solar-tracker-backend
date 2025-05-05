#pragma once

#include <Arduino.h>
#include <LCD_I2C.h>
#include "enum.h"

struct MutableLCDState
{
    byte row;
    String text;
    bool selectable = true;

    MutableLCDState(byte r, String t) : row(r), text(t) {}

    void generateText(LCD_I2C &lcd, bool selected = false) const
    {
        lcd.print((selected ? ">" : " ") + text);
    }

    void generateTextValue(LCD_I2C &lcd, float value, bool selected = false) const
    {
        lcd.print((selected ? ">" : " ") + text + ": " + String(value));
    }
};

class UserInterface
{
private:
    int currentRow = 0;
    int cursorIndex = 0;
    bool moveUpward = false; // <-- Toggle direction
    LCD_I2C lcd;

    static const uint8_t maxIdleStates = 2;
    MutableLCDState idleList[maxIdleStates] = {
        {1, "Manual"},
        {2, "Auto"},
    };
    static const uint8_t maxManualStates = 3;
    MutableLCDState manualList[maxManualStates] = {
        {1, "X"},
        {2, "Y"},
        {3, "Back"},
    };

public:
    UserInterface() : lcd(0x27, 20, 4) {};
    ~UserInterface() {};

    UserInterfaceState currentState = UserInterfaceState::IDLE;

    void init()
    {
        lcd.begin(20, 4);
        lcd.backlight();
        lcd.clear();
    }

    void changePage()
    {
        if (currentRow == 1)
        {
            currentState = UserInterfaceState::MANUAL;
            cursorIndex = 0;
            currentRow = manualList[cursorIndex].row;
        }
        else if (currentRow == 2)
        {
            currentState = UserInterfaceState::IDLE;
            cursorIndex = 0;
            currentRow = idleList[cursorIndex].row;
        }
        else if (currentRow == 3)
        {
            currentState = UserInterfaceState::IDLE;
            cursorIndex = 0;
            currentRow = idleList[cursorIndex].row;
        }
    }

    void resetRow()
    {
        currentRow = 0;
        cursorIndex = 0;
    }

    void moveCursor(bool upward)
    {
        MutableLCDState *list = nullptr;
        int maxStates = 0;

        switch (currentState)
        {
        case UserInterfaceState::IDLE:
            list = idleList;
            maxStates = maxIdleStates;
            break;
        case UserInterfaceState::MANUAL:
            list = manualList;
            maxStates = maxManualStates;
            break;
        // Extend here if AUTOMATIC uses a menu too
        default:
            return;
        }

        int dir = upward ? -1 : 1;
        int newIndex = cursorIndex + dir;

        // Clamp
        if (newIndex < 0)
            newIndex = 0;
        else if (newIndex >= maxStates)
            newIndex = maxStates - 1;

        while (!list[newIndex].selectable)
        {
            newIndex += dir;

            if (newIndex < 0 || newIndex >= maxStates)
                return; // Nothing selectable in this direction
        }

        cursorIndex = newIndex;
        currentRow = list[cursorIndex].row;

        Serial.print("Row: ");
        Serial.println(currentRow);
    }

    void initDisplay()
    {
        lcd.clear();
        lcd.setCursor(6, 0);
        lcd.print("Solar Tracker");
        lcd.setCursor(3, 1);
        lcd.print("Initializing...");
    }

    void idleDisplay(byte sunValue, float angleX, float angleY)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Mode: IDLE");
        lcd.setCursor(0, 1);
        lcd.print("X: ");
        lcd.print(angleX);
        lcd.setCursor(0, 2);
        lcd.print("Y: ");
        lcd.print(angleY);
        lcd.setCursor(0, 3);
        lcd.print("Sun: ");
        lcd.print(sunValue);

        for (int i = 0; i < maxIdleStates; i++)
        {
            lcd.setCursor(10, idleList[i].row);
            idleList[i].generateText(lcd, i == cursorIndex);
        }
    }

    void manualDisplay(byte sunValue, float angleX, float angleY)
    {
        lcd.clear();
        lcd.setCursor(0, 0);

        lcd.print("Mode: MANUAL");
        lcd.setCursor(0, 1);
        for (int i = 0; i < maxManualStates; i++)
        {
            lcd.setCursor(10, manualList[i].row);
            manualList[i].generateText(lcd, i == cursorIndex);
        }
    }
};

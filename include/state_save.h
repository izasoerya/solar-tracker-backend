#pragma once

#include <EEPROM.h>
#include <user_interface.h>

struct SystemStructure
{
    AppState state;
    ManualSelection selection;
    bool inEditMode;
    int xVal;
    int yVal;
};

class StateSave
{
private:
    SystemStructure _structure;

public:
    StateSave();
    ~StateSave();

    void initialization();
    SystemStructure getState();
    void updateState(SystemStructure newState);
};

StateSave::StateSave() {}
StateSave::~StateSave() {}

void StateSave::initialization()
{
    bool isEmpty = true;
    for (int i = 0; i < 5; i++)
    {
        if (EEPROM.read(i) != 255)
        {
            isEmpty = false;
            break;
        }
    }

    if (isEmpty)
    {
        _structure.state = AppState::AUTOMATIC;
        _structure.selection = ManualSelection::X;
        _structure.inEditMode = false;
        _structure.xVal = 0;
        _structure.yVal = 0;

        EEPROM.write(0, 0); // AppState::AUTOMATIC
        EEPROM.write(1, 0); // ManualSelection::X
        EEPROM.write(2, 0); // inEditMode false
        EEPROM.write(3, 0); // xVal
        EEPROM.write(4, 0); // yVal
    }
    else
    {
        byte _tempAppstate = EEPROM.read(0);
        if (_tempAppstate == 0)
            _structure.state = AppState::AUTOMATIC;
        else if (_tempAppstate == 1)
            _structure.state = AppState::MANUAL;

        byte _tempSelection = EEPROM.read(1);
        if (_tempSelection == 0)
            _structure.selection = ManualSelection::X;
        else if (_tempSelection == 1)
            _structure.selection = ManualSelection::Y;
        else if (_tempSelection == 2)
            _structure.selection = ManualSelection::BACK;
        else if (_tempSelection == 3)
            _structure.selection = ManualSelection::COUNT;

        byte _tempEditMode = EEPROM.read(2);
        _structure.inEditMode = (_tempEditMode == 1);

        int8_t _tempXval = EEPROM.read(3);
        _structure.xVal = (_tempXval == 255) ? 0 : _tempXval;

        int8_t _tempYval = EEPROM.read(4);
        _structure.yVal = (_tempYval == 255) ? 0 : _tempYval;
    }
}

SystemStructure StateSave::getState()
{
    return _structure;
}

void StateSave::updateState(SystemStructure newState)
{
    _structure = newState;

    // Write each field to EEPROM
    EEPROM.write(0, static_cast<uint8_t>(_structure.state));
    EEPROM.write(1, static_cast<uint8_t>(_structure.selection));
    EEPROM.write(2, _structure.inEditMode ? 1 : 0);
    EEPROM.write(3, static_cast<int8_t>(_structure.xVal));
    EEPROM.write(4, static_cast<int8_t>(_structure.yVal));
}
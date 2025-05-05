#pragma once

#include <Arduino.h>

enum class UserInterfaceState
{
    IDLE,
    AUTOMATIC,
    MANUAL,
};

// Helper function to convert UserInterfaceState to String
inline String toString(UserInterfaceState state)
{
    switch (state)
    {
    case UserInterfaceState::IDLE:
        return "IDLE";
    case UserInterfaceState::AUTOMATIC:
        return "AUTOMATIC";
    case UserInterfaceState::MANUAL:
        return "MANUAL";
    default:
        return "UNKNOWN";
    }
}
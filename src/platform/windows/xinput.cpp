#include "xinput.hpp"

XInputGetState_t _XInputGetState = nullptr;

$execute {
    // TODO: maybe load xinput not like this
    _XInputGetState = (XInputGetState_t)GetProcAddress(LoadLibrary("XInput1_4.dll"), "XInputGetState");
}

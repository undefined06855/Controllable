#include "xinput.hpp"

XInputGetState_t _XInputGetState = nullptr;

$execute {
    auto xinput = GetModuleHandle("XInput1_4.dll");
    if (!xinput) {
        xinput = GetModuleHandle("XInput1_4.dll");
    }

    if (!xinput) {
        geode::log::error("no xinput what the fuck did you do");
    }

    _XInputGetState = (XInputGetState_t)GetProcAddress(xinput, "XInputGetState");
}

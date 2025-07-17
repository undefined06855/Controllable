#include "xinput.hpp"

XInputGetState_t _XInputGetState = nullptr;
XInputSetState_t _XInputSetState = nullptr;

$execute {
    auto xinput = GetModuleHandle("XInput1_4.dll");
    if (!xinput) {
        xinput = GetModuleHandle("Xinput9_1_0.dll");
    }

    if (!xinput) {
        geode::log::error("no xinput what the fuck did you do");
    }

    _XInputGetState = (XInputGetState_t)GetProcAddress(xinput, "XInputGetState");
    _XInputSetState = (XInputSetState_t)GetProcAddress(xinput, "XInputSetState");
}

#include "xinput.hpp"

XInputGetState_t _XInputGetState = nullptr;

$execute {
    auto xinput = GetModuleHandle("XInput1_4.dll");
    if (!xinput) {
        xinput = GetModuleHandle("XInput1_4.dll");
    }

    if (!xinput) {
        geode::log::warn("lol no xinput");
    }

    _XInputGetState = (XInputGetState_t)GetProcAddress(xinput, "XInputGetState");
}

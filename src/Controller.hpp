#pragma once
#include "utils.hpp"

class Controller {
public:
    Controller();

    XINPUT_STATE m_state;
    Direction m_lastDirection;
    GamepadButton m_lastGamepadButton;

    void update();

    Direction directionJustPressed();
    GamepadButton gamepadButtonJustPressed();
    GamepadButton gamepadButtonJustReleased();

    Direction directionPressed();
    GamepadButton gamepadButtonPressed();

    float getRightJoyY();
};
extern Controller g_controller;

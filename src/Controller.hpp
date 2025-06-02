#pragma once
#include "enums.hpp"

struct ControllerState {
    bool m_buttonA, m_buttonB, m_buttonX, m_buttonY,
         m_buttonStart, m_buttonSelect,
         m_buttonL, m_buttonR, m_buttonZL, m_buttonZR,
         m_buttonUp, m_buttonDown, m_buttonLeft, m_buttonRight,
         m_joyLeft, m_joyRight;
    float m_joyLeftX, m_joyLeftY,
          m_joyRightX, m_joyRightY;
};

// note that implementation of this is platform-specific!

class Controller {
public:
    Controller();

    ControllerState m_state;
    GamepadDirection m_lastDirection;
    GamepadButton m_lastGamepadButton;
    bool m_connected;

    void update();

    GamepadDirection directionJustPressed();
    GamepadDirection directionJustReleased();
    GamepadButton gamepadButtonJustPressed();
    GamepadButton gamepadButtonJustReleased();

    GamepadDirection directionPressed();
    GamepadButton gamepadButtonPressed();

    cocos2d::CCPoint getLeftJoystick();
    cocos2d::CCPoint getRightJoystick();
};
extern Controller g_controller;

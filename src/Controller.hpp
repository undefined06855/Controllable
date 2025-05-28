#pragma once
#include "enums.hpp"

struct ControllerState {
    bool m_buttonA, m_buttonB, m_buttonX, m_buttonY,
         m_buttonStart, m_buttonSelect,
         m_buttonL, m_buttonR, m_buttonZL, m_buttonZR,
         m_buttonUp, m_buttonDown, m_buttonLeft, m_buttonRight;
    float m_joyLeftX, m_joyLeftY,
          m_joyRightX, m_joyRightY;
};

class Controller {
public:
    Controller();

    ControllerState m_state;
    Direction m_lastDirection;
    GamepadButton m_lastGamepadButton;

    void update();

    Direction directionJustPressed();
    Direction directionJustReleased();
    GamepadButton gamepadButtonJustPressed();
    GamepadButton gamepadButtonJustReleased();

    Direction directionPressed();
    GamepadButton gamepadButtonPressed();

    cocos2d::CCPoint getLeftJoystick();
    cocos2d::CCPoint getRightJoystick();
};
extern Controller g_controller;

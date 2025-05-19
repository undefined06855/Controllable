#pragma once
#include "utils.hpp"

struct ControllerState {
    bool buttonA, buttonB, buttonX, buttonY,
         buttonStart, buttonSelect,
         buttonL, buttonR, buttonZL, buttonZR,
         buttonUp, buttonDown, buttonLeft, buttonRight;
    float joyLeftX, joyLeftY,
          joyRightX, joyRightY;
};

class Controller {
public:
    Controller();

    ControllerState m_state;
    Direction m_lastDirection;
    GamepadButton m_lastGamepadButton;

    void update();

    Direction directionJustPressed();
    GamepadButton gamepadButtonJustPressed();
    GamepadButton gamepadButtonJustReleased();

    Direction directionPressed();
    GamepadButton gamepadButtonPressed();

    cocos2d::CCPoint getLeftJoystick();
    cocos2d::CCPoint getRightJoystick();
};
extern Controller g_controller;
